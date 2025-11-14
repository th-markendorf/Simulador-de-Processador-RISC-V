#include "Core.h"
#include <array>
#include <iomanip>
#include <limits>
#include <ostream>
#include <ranges>

#include "Instruction.h"

Core::Core(size_t tamanho_memoria) : memoria(tamanho_memoria, 0) {
    cache = std::make_unique<Cache>(4096, 16, memoria);
    reset();
}

void Core::reset() {
    contador_programa = 0x0;
    for (unsigned int &registradore: registradores) {
        registradore = 0;
    }
    cache->reset();

    // Limpa os registradores de pipeline
    reg_if_id = {};
    reg_id_ex = {};
    reg_ex_mem = {};
    reg_mem_wb = {};
    m_pipeline_stall = false;
}

bool Core::is_finished() const {
    return contador_programa >= memoria.size();
}

void Core::load_program(const std::vector<uint32_t> &programa) {
    // Copia um programa (vetor de instruções) para a memória principal
    for (size_t i = 0; i < programa.size(); ++i) {
        memoria[i * 4 + 0] = (programa[i] >> 0) & 0xFF;
        memoria[i * 4 + 1] = (programa[i] >> 8) & 0xFF;
        memoria[i * 4 + 2] = (programa[i] >> 16) & 0xFF;
        memoria[i * 4 + 3] = (programa[i] >> 24) & 0xFF;
    }
}

std::array<uint32_t, 32> Core::get_registradores() const {
    // Retorna uma cópia do banco de registradores para a GUI
    std::array<uint32_t, 32> regs{};
    std::ranges::copy(registradores, std::begin(regs));
    return regs;
}

uint32_t Core::get_program_counter() const {
    // Retorna o PC atual para a GUI
    return contador_programa;
}

std::string Core::set_register(int reg_index, uint32_t valor) {
    // Permite que a GUI (DemoWindow) defina valores nos registradores
    if (reg_index > 0 && reg_index < 32) {
        registradores[reg_index] = valor;
        return "";
    } else if (reg_index == 0) {
        return "[AVISO] Nao e permitido alterar o registrador x0 (zero).";
    } else {
        std::stringstream ss;
        ss << "[ERRO] Tentativa de acessar registrador invalido: " << reg_index;
        return ss.str();
    }
}

uint8_t Core::get_byte_memoria(uint32_t endereco) const {
    // Permite que a GUI (MainWindow) leia bytes individuais da memória
    if (endereco >= memoria.size()) {
        return 0;
    }
    return memoria[endereco];
}

// --- Lógica Principal do Pipeline ---
void Core::tick_clock() {
    // Executa os 5 estágios do pipeline em ordem inversa (para propagar os dados corretamente)
    pipeline_estagio_WB();
    pipeline_estagio_MEM();
    pipeline_estagio_EX();
    pipeline_estagio_ID();
    pipeline_estagio_IF();
}

// Estágio 5 (WB): Escreve o resultado de volta aos registradores
void Core::pipeline_estagio_WB() {
    // Se a instrução vinda não for válida, não faz nada
    if (!reg_mem_wb.valido) {
        return;
    }

    if (reg_mem_wb.escrever_registrador) {
        uint32_t rd = reg_mem_wb.rd_destino;

        // Proteção para nunca escrever no registrador x0
        if (rd != 0) {
            uint32_t valor_final;

            if (reg_mem_wb.mem_para_reg) {
                valor_final = reg_mem_wb.dado_lido_da_memoria;
            } else {
                valor_final = reg_mem_wb.resultado_ula;
            }

            // Escreve o valor final no banco de registradores
            registradores[rd] = valor_final;
        }
    }
}

// Estágio 4 (MEM): Acessa a memória de dados (Leitura/Escrita)
void Core::pipeline_estagio_MEM() {
    if (!reg_ex_mem.valido) {
        reg_mem_wb.valido = false;
        return;
    }

    // Passa os sinais de controle e dados adiante para o estágio WB
    reg_mem_wb.valido = true;
    reg_mem_wb.escrever_registrador = reg_ex_mem.escrever_registrador;
    reg_mem_wb.rd_destino = reg_ex_mem.rd_destino;
    reg_mem_wb.resultado_ula = reg_ex_mem.resultado_ula;
    reg_mem_wb.mem_para_reg = reg_ex_mem.mem_para_reg;

    // O endereço de acesso à memória foi calculado pela ULA no estágio EX
    uint32_t endereco = reg_ex_mem.resultado_ula;

    // Se for uma instrução de Leitura (LW)
    if (reg_ex_mem.ler_memoria) {
        // Lê do cache e armazena no registrador de pipeline
        reg_mem_wb.dado_lido_da_memoria = cache->lerDados(endereco);
    }

    // Se for uma instrução de Escrita (SW)
    if (reg_ex_mem.escrever_memoria) {
        // Escreve o valor (vindo de rs2) no cache
        uint32_t valor = reg_ex_mem.valor_para_escrever_memoria;
        cache->escreverDados(endereco, valor);
    }
}

// Estágio 3 (EX): Executa a operação na ULA
void Core::pipeline_estagio_EX() {
    // Se a instrução vinda do estágio ID não for válida (bolha), propaga a bolha
    if (!reg_id_ex.valido) {
        reg_ex_mem.valido = false;
        return;
    }

    // Passa os sinais de controle e dados adiante para o estágio MEM
    reg_ex_mem.valido = true;
    reg_ex_mem.ler_memoria = reg_id_ex.ler_memoria;
    reg_ex_mem.escrever_memoria = reg_id_ex.escrever_memoria;
    reg_ex_mem.escrever_registrador = reg_id_ex.escrever_registrador;
    reg_ex_mem.rd_destino = reg_id_ex.rd_destino;
    reg_ex_mem.valor_para_escrever_memoria = reg_id_ex.valor_rs2; // Para SW
    reg_ex_mem.mem_para_reg = reg_id_ex.mem_para_reg; // Para WB

    // Valor base: Pegamos do banco de registradores "agora".
    // Isso resolve automaticamente o Hazard de distância 2 (WB), pois o estágio WB
    // roda antes do EX no seu tick_clock(), então registradores[] já está atualizado.
    int32_t rs1_atual = registradores[reg_id_ex.rs1_index];
    int32_t rs2_atual = registradores[reg_id_ex.rs2_index];

    // Verifica Forwarding do estágio MEM (Hazard de distância 1)
    // A instrução que estava em EX no ciclo passado agora está em reg_mem_wb (pois MEM já rodou)
    bool forward_rs1_mem = (reg_mem_wb.valido && reg_mem_wb.escrever_registrador && reg_mem_wb.rd_destino != 0 && reg_mem_wb.rd_destino == reg_id_ex.rs1_index);
    bool forward_rs2_mem = (reg_mem_wb.valido && reg_mem_wb.escrever_registrador && reg_mem_wb.rd_destino != 0 && reg_mem_wb.rd_destino == reg_id_ex.rs2_index);

    if (forward_rs1_mem) {
        // Se a instrução anterior era um Load, o dado é 'dado_lido', senão é 'resultado_ula'
        rs1_atual = reg_mem_wb.mem_para_reg ? reg_mem_wb.dado_lido_da_memoria : reg_mem_wb.resultado_ula;
    }

    if (forward_rs2_mem) {
        rs2_atual = reg_mem_wb.mem_para_reg ? reg_mem_wb.dado_lido_da_memoria : reg_mem_wb.resultado_ula;
    }

    // Atualiza o valor para SW (Store Word) com o dado correto (forwarded)
    reg_ex_mem.valor_para_escrever_memoria = rs2_atual;

    // --- Seleção de Operandos da ULA ---
    int32_t operando1 = rs1_atual;
    int32_t operando2;

    // O operando2 é o imediato ou o valor de rs2 (atualizado)
    // Lista de opcodes que usam Imediato (Tipo-I, Load, Store, LUI, JAL, JALR)
    bool usa_imediato = (reg_id_ex.opcode == 0x13 || reg_id_ex.opcode == 0x03 || reg_id_ex.opcode == 0x23 || reg_id_ex.opcode == 0x37 || reg_id_ex.opcode == 0x6F);

    if (usa_imediato) {
        operando2 = reg_id_ex.imediato;
    } else {
        operando2 = rs2_atual;
    }

    if (reg_id_ex.usar_ula) {
        // Executa a operação baseada no sinal de controle vindo do ID
        switch (reg_id_ex.operacao_ula) {
            // Base: ADDI, ADD, LW, SW
            case ULA_SOMA:
                reg_ex_mem.resultado_ula = operando1 + operando2;
                break;
            // Base: SUB
            case ULA_SUB:
                reg_ex_mem.resultado_ula = operando1 - operando2;
                break;
            // Base: XORI, XOR
            case ULA_XOR:
                reg_ex_mem.resultado_ula = operando1 ^ operando2;
                break;
            // Base: ORI, OR
            case ULA_OR:
                reg_ex_mem.resultado_ula = operando1 | operando2;
                break;
            // Base: ANDI, AND
            case ULA_AND:
                reg_ex_mem.resultado_ula = operando1 & operando2;
                break;
            // Base: SLLI, SLL
            case ULA_SLL:
                reg_ex_mem.resultado_ula = operando1 << (operando2 & 0x1F);
                break;
            // Base: SRLI, SRL (Lógico)
            case ULA_SRL:
                reg_ex_mem.resultado_ula = static_cast<uint32_t>(operando1) >> (operando2 & 0x1F);
                break;
            // Base: SRAI, SRA (Aritmético)
            case ULA_SRA:
                reg_ex_mem.resultado_ula = operando1 >> (operando2 & 0x1F);
                break;
            // Base: SLTI, SLT (Signed)
            case ULA_SLT:
                reg_ex_mem.resultado_ula = (operando1 < operando2) ? 1 : 0;
                break;
            // Base: SLTIU, SLTU (Unsigned)
            case ULA_SLTU:
                reg_ex_mem.resultado_ula = (static_cast<uint32_t>(operando1) < static_cast<uint32_t>(operando2))
                                               ? 1
                                               : 0;
                break;
            // M: MUL
            case ULA_MUL:
                reg_ex_mem.resultado_ula = static_cast<uint32_t>(
                    (static_cast<int64_t>(operando1) * static_cast<int64_t>(operando2)) & 0xFFFFFFFF);
                break;
            // M: MULH (Signed * Signed)
            case ULA_MULH:
                reg_ex_mem.resultado_ula = static_cast<uint32_t>(
                    (static_cast<int64_t>(operando1) * static_cast<int64_t>(operando2)) >> 32);
                break;
            // M: MULHSU (Signed * Unsigned)
            case ULA_MULHSU:
                reg_ex_mem.resultado_ula = static_cast<uint32_t>(
                    (static_cast<int64_t>(operando1) * static_cast<uint64_t>(static_cast<uint32_t>(operando2))) >> 32);
                break;
            // M: MULHU (Unsigned * Unsigned)
            case ULA_MULHU:
                reg_ex_mem.resultado_ula = static_cast<uint32_t>(
                    (static_cast<uint64_t>(static_cast<uint32_t>(operando1)) * static_cast<uint64_t>(static_cast<
                         uint32_t>(
                         operando2))) >> 32);
                break;
            // M: DIV (Signed)
            case ULA_DIV:
                if (operando2 == 0) reg_ex_mem.resultado_ula = 0xFFFFFFFF; // Divisão por zero
                else if (operando1 == std::numeric_limits<int32_t>::min() && operando2 == -1)
                    reg_ex_mem.resultado_ula = operando1; // Overflow
                else reg_ex_mem.resultado_ula = operando1 / operando2;
                break;
            // M: DIVU (Unsigned)
            case ULA_DIVU:
                if (operando2 == 0) reg_ex_mem.resultado_ula = 0xFFFFFFFF; // Divisão por zero
                else reg_ex_mem.resultado_ula = static_cast<uint32_t>(operando1) / static_cast<uint32_t>(operando2);
                break;
            // M: REM (Signed)
            case ULA_REM:
                if (operando2 == 0) reg_ex_mem.resultado_ula = operando1; // Divisão por zero
                else if (operando1 == std::numeric_limits<int32_t>::min() && operando2 == -1)
                    reg_ex_mem.resultado_ula = 0;
                    // Overflow
                else reg_ex_mem.resultado_ula = operando1 % operando2;
                break;
            // M: REMU (Unsigned)
            case ULA_REMU:
                if (operando2 == 0) reg_ex_mem.resultado_ula = operando1; // Divisão por zero
                else reg_ex_mem.resultado_ula = static_cast<uint32_t>(operando1) % static_cast<uint32_t>(operando2);
                break;
            // Base: LUI
            case ULA_LUI:
                reg_ex_mem.resultado_ula = operando2; // operando2 é o imediato
                break;
            // NOP ou operação desconhecida
            case ULA_NOP:
            default:
                reg_ex_mem.resultado_ula = 0;
                break;
        }
    }

    // Se a instrução for um JAL (sempre tomado)
    if (reg_id_ex.eh_jal) {
        m_do_flush = true;
        m_new_pc_target = reg_id_ex.pc + reg_id_ex.imediato;
    }
    // Se a instrução for um Branch (precisa decidir)
    else if (reg_id_ex.eh_branch) {
        bool tomar_desvio = false;

        // Compara operando1 (valor_rs1) e operando2 (valor_rs2)
        switch (reg_id_ex.funct3_branch) {
            case 0x0: // BEQ (Branch if Equal)
                if (operando1 == operando2) tomar_desvio = true;
                break;
            case 0x1: // BNE (Branch if Not Equal)
                if (operando1 != operando2) tomar_desvio = true;
                break;

            case 0x4: // BLT (Branch if Less Than, Signed)
                if (operando1 < operando2) tomar_desvio = true;
                break;
            case 0x5: // BGE (Branch if Greater or Equal, Signed)
                if (operando1 >= operando2) tomar_desvio = true;
                break;
            case 0x6: // BLTU (Branch if Less Than, Unsigned)
                if (static_cast<uint32_t>(operando1) < static_cast<uint32_t>(operando2)) tomar_desvio = true;
                break;
            case 0x7: // BGEU (Branch if Greater or Equal, Unsigned)
                if (static_cast<uint32_t>(operando1) >= static_cast<uint32_t>(operando2)) tomar_desvio = true;
                break;
        }

        // Se a condição for verdadeira, aciona o flush
        if (tomar_desvio) {
            m_do_flush = true;
            m_new_pc_target = reg_id_ex.pc + reg_id_ex.imediato;
        }
    }
}

// Estágio 2 (ID): Decodifica a instrução e detecta hazards
void Core::pipeline_estagio_ID() {
    // Se o estágio EX (no ciclo anterior) acionou um flush,
    // esta instrução (em ID) deve ser cancelada (transformada em bolha).
    if (m_do_flush) {
        reg_id_ex = {};
        reg_id_ex.valido = false;
        return; // Não faz mais nada neste estágio
    }

    // Lê a instrução bruta vinda do estágio IF
    uint32_t instrucao_bruta = reg_if_id.instrucao;
    Instruction inst(instrucao_bruta); // Decodifica

    // Verifica se a instrução no estágio EX é um LW (ler_memoria)
    // E se o destino dela (rd) é uma das fontes (rs1 ou rs2) da instrução ATUAL (em ID)
    if (reg_id_ex.ler_memoria && reg_id_ex.rd_destino != 0 &&
        (reg_id_ex.rd_destino == inst.rs1() || reg_id_ex.rd_destino == inst.rs2())) {
        // 1. Injeta uma "bolha" (NOP) no estágio EX
        reg_id_ex = {};
        reg_id_ex.valido = false;
        // 2. Aciona o "freio" (stall) para o estágio IF e o PC
        m_pipeline_stall = true;
        return;
    }

    // Se o estágio IF não passou uma instrução válida...
    if (!reg_if_id.valido) {
        reg_id_ex.valido = false;
        return;
    }

    // Reseta todos os sinais de controle para o próximo estágio
    reg_id_ex = {};
    reg_id_ex.valido = true;
    reg_id_ex.pc = reg_if_id.pc;
    reg_id_ex.opcode = inst.opcode(); // Salva o opcode (para o EX)

    reg_id_ex.rs1_index = inst.rs1(); // Salva o índice (ex: x1)
    reg_id_ex.rs2_index = inst.rs2(); // Salva o índice (ex: x2)

    // Lê os valores dos registradores fonte (rs1 e rs2)
    reg_id_ex.valor_rs1 = registradores[inst.rs1()];
    reg_id_ex.valor_rs2 = registradores[inst.rs2()];

    // Salva o registrador de destino
    reg_id_ex.rd_destino = inst.rd();

    // Define os sinais de controle baseado no opcode da instrução
    switch (inst.opcode()) {
        case 0x13: // --- Tipo-I ---
            reg_id_ex.usar_ula = true;
            reg_id_ex.escrever_registrador = true;
            reg_id_ex.imediato = inst.imediato_tipo_I();
            switch (inst.funct3()) {
                case 0x0: reg_id_ex.operacao_ula = ULA_SOMA;
                    break;
                case 0x1: reg_id_ex.operacao_ula = ULA_SLL;
                    break;
                case 0x2: reg_id_ex.operacao_ula = ULA_SLT;
                    break;
                case 0x3: reg_id_ex.operacao_ula = ULA_SLTU;
                    break;
                case 0x4: reg_id_ex.operacao_ula = ULA_XOR;
                    break;
                case 0x5:
                    reg_id_ex.operacao_ula = (inst.funct7() == 0x00) ? ULA_SRL : ULA_SRA;
                    break;
                case 0x6: reg_id_ex.operacao_ula = ULA_OR;
                    break;
                case 0x7: reg_id_ex.operacao_ula = ULA_AND;
                    break;
                default: reg_id_ex.operacao_ula = ULA_NOP;
                    break;
            }
            break;

        case 0x33: // --- Tipo-R ---
            reg_id_ex.usar_ula = true;
            reg_id_ex.escrever_registrador = true;
            if (inst.funct7() == 0x01) {
                // Extensão M
                switch (inst.funct3()) {
                    case 0x0: reg_id_ex.operacao_ula = ULA_MUL;
                        break;
                    case 0x1: reg_id_ex.operacao_ula = ULA_MULH;
                        break;
                    case 0x2: reg_id_ex.operacao_ula = ULA_MULHSU;
                        break;
                    case 0x3: reg_id_ex.operacao_ula = ULA_MULHU;
                        break;
                    case 0x4: reg_id_ex.operacao_ula = ULA_DIV;
                        break;
                    case 0x5: reg_id_ex.operacao_ula = ULA_DIVU;
                        break;
                    case 0x6: reg_id_ex.operacao_ula = ULA_REM;
                        break;
                    case 0x7: reg_id_ex.operacao_ula = ULA_REMU;
                        break;
                    default: reg_id_ex.operacao_ula = ULA_NOP;
                        break;
                }
            } else {
                // Base
                switch (inst.funct3()) {
                    case 0x0:
                        reg_id_ex.operacao_ula = (inst.funct7() == 0x00) ? ULA_SOMA : ULA_SUB;
                        break;
                    case 0x1: reg_id_ex.operacao_ula = ULA_SLL;
                        break;
                    case 0x2: reg_id_ex.operacao_ula = ULA_SLT;
                        break;
                    case 0x3: reg_id_ex.operacao_ula = ULA_SLTU;
                        break;
                    case 0x4: reg_id_ex.operacao_ula = ULA_XOR;
                        break;
                    case 0x5:
                        reg_id_ex.operacao_ula = (inst.funct7() == 0x00) ? ULA_SRL : ULA_SRA;
                        break;
                    case 0x6: reg_id_ex.operacao_ula = ULA_OR;
                        break;
                    case 0x7: reg_id_ex.operacao_ula = ULA_AND;
                        break;
                    default: reg_id_ex.operacao_ula = ULA_NOP;
                        break;
                }
            }
            break;

        case 0x03: // --- Load (LW) ---
            reg_id_ex.usar_ula = true;
            reg_id_ex.ler_memoria = true;
            reg_id_ex.escrever_registrador = true;
            reg_id_ex.mem_para_reg = true;
            reg_id_ex.imediato = inst.imediato_tipo_I();
            reg_id_ex.operacao_ula = ULA_SOMA;
            break;

        case 0x23: // --- Store (SW) ---
            reg_id_ex.usar_ula = true;
            reg_id_ex.escrever_memoria = true;
            reg_id_ex.escrever_registrador = false;
            reg_id_ex.imediato = inst.imediato_tipo_S();
            reg_id_ex.operacao_ula = ULA_SOMA;
            break;

        // --- Desvios ---
        case 0x37: // --- LUI ---
            reg_id_ex.usar_ula = true;
            reg_id_ex.escrever_registrador = true;
            reg_id_ex.rd_destino = inst.rd();
            reg_id_ex.imediato = inst.imediato_tipo_U();
            reg_id_ex.operacao_ula = ULA_LUI;
            break;

        case 0x6F: // --- JAL ---
            reg_id_ex.usar_ula = true;
            reg_id_ex.escrever_registrador = true;
            reg_id_ex.rd_destino = inst.rd();
            reg_id_ex.imediato = inst.imediato_tipo_J();
            reg_id_ex.operacao_ula = ULA_SOMA;
            reg_id_ex.valor_rs1 = reg_if_id.pc;
            reg_id_ex.valor_rs2 = 4;
            reg_id_ex.eh_jal = true;
            break;

        case 0x63: // --- BRANCH ---
            reg_id_ex.usar_ula = false;
            reg_id_ex.escrever_registrador = false;
            reg_id_ex.imediato = inst.imediato_tipo_B();
            reg_id_ex.eh_branch = true;
            reg_id_ex.funct3_branch = inst.funct3();
            break;

        default:
            reg_id_ex.valido = false; // Injeta uma bolha
            break;
    }
}

// Estágio 1 (IF): Busca da Instrução
void Core::pipeline_estagio_IF() {
    // 1. Flush (Hazard de Controle)
    // Se um desvio foi sinalizado (m_do_flush), cancela a instrução atual.
    if (m_do_flush) {
        m_do_flush = false;
        contador_programa = m_new_pc_target; // Atualiza PC para o alvo

        // Injeta uma bolha
        reg_if_id = {};
        reg_if_id.valido = false;
        return; // Sai do estágio
    }

    // 2. Stall (Hazard de Dados / Load-Use)
    // Se o ID sinalizou um stall (m_pipeline_stall), congela o IF e o PC.
    if (m_pipeline_stall) {
        m_pipeline_stall = false; // Reseta o sinal

        // Mantém o PC e o IF_ID atuais, permitindo a bolha avançar.
        return;
    }

    // 3. Busca Normal
    // Busca a instrução do PC atual via cache.
    uint32_t instrucao = cache->lerDados(contador_programa);

    // Passa a instrução e o PC para o próximo estágio.
    reg_if_id.instrucao = instrucao;
    reg_if_id.pc = contador_programa;
    reg_if_id.valido = true;

    // Avança o PC (PC = PC + 4)
    contador_programa += 4;
}