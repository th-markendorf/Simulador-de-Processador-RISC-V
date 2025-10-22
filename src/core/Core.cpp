#include "Core.h"

#include <array>
#include <iomanip>
#include <ostream>

Core::Core(size_t tamanho_memoria) : memoria(tamanho_memoria, 0) {
    cache = std::make_unique<Cache>(4096, 16, memoria);
    reset();
}

void Core::reset() {
    contador_programa = 0x0;
    for (unsigned int & registradore : registradores) {
        registradore = 0;
    }
    cache->reset();
}

bool Core::is_finished() const {
    return contador_programa >= memoria.size();
}

std::string Core::step() {
    if (is_finished()) {
        std::stringstream ss;
        ss << "Simulacao ja finalizada. PC em 0x" << std::hex << contador_programa;
        return ss.str();
    }

    uint32_t instrucao = fetch();

    std::string log = execute(instrucao);
    return log;
}

void Core::load_program(const std::vector<uint32_t> &programa) {
    for (size_t i = 0; i < programa.size(); ++i) {
        memoria[i * 4 + 0] = (programa[i] >> 0) & 0xFF;
        memoria[i * 4 + 1] = (programa[i] >> 8) & 0xFF;
        memoria[i * 4 + 2] = (programa[i] >> 16) & 0xFF;
        memoria[i * 4 + 3] = (programa[i] >> 24) & 0xFF;
    }
}

uint32_t Core::fetch() {
    return cache->lerDados(contador_programa);
}

std::string Core::execute(uint32_t instrucao) {
    // Decodifica a instrução
    Instruction inst(instrucao);
    std::stringstream log_ss;

    if (inst.palavra_instrucao == 0) {
        contador_programa = memoria.size();
        return "Instrucao nula, finalizando.";
    }

    std::string log_msg;

    // Despacha para o handler correto baseado no opcode
    switch (inst.opcode()) {
        case 0x13: log_msg = handle_op_imm(inst);
            break;
        case 0x33: log_msg = handle_op_reg(inst);
            break;
        case 0x03: log_msg = handle_load(inst);
            break;
        case 0x23: log_msg = handle_store(inst);
            break;
        case 0x63: log_msg = handle_branch(inst);
            break;
        case 0x6F: log_msg = handle_jal(inst);
            break;
        case 0x37: log_msg = handle_lui(inst);
            break;

        default:
            log_ss << "ERRO: Opcode desconhecido: 0x" << std::hex << inst.opcode();
            contador_programa += 4; // Avança para não travar
            log_msg = log_ss.str();
            break;
    }

    // 3. Garante que x0 seja sempre zero após cada instrução
    registradores[0] = 0;

    return log_msg;
}

std::string Core::set_register(int reg_index, uint32_t valor) {
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

std::array<uint32_t, 32> Core::get_registradores() const {
    std::array<uint32_t, 32> regs{};
    std::ranges::copy(registradores, std::begin(regs));
    return regs;
}

uint32_t Core::get_program_counter() const {
    return contador_programa;
}

std::string Core::handle_op_imm(const Instruction &inst) {
    std::stringstream log_ss;

    int32_t imm = inst.imediato_tipo_I();
    uint32_t rd = inst.rd();
    uint32_t rs1 = inst.rs1();

    switch (inst.funct3()) {
        case 0x0: // ADDI
            log_ss << "Executando ADDI x" << std::dec << rd << ", x" << rs1 << ", " << imm;
            if (rd != 0) registradores[rd] = registradores[rs1] + imm;
            break;
        case 0x2: // SLTI
            log_ss << "Executando SLTI x" << std::dec << rd << ", x" << rs1 << ", " << imm;
            if (rd != 0) registradores[rd] = (static_cast<int32_t>(registradores[rs1]) < imm) ? 1 : 0;
            break;
        case 0x4: // XORI
            log_ss << "Executando XORI x" << std::dec << rd << ", x" << rs1 << ", " << imm;
            if (rd != 0) registradores[rd] = registradores[rs1] ^ imm;
            break;
        case 0x6: // ORI
            log_ss << "Executando ORI x" << std::dec << rd << ", x" << rs1 << ", " << imm;
            if (rd != 0) registradores[rd] = registradores[rs1] | imm;
            break;
        case 0x7: // ANDI
            log_ss << "Executando ANDI x" << std::dec << rd << ", x" << rs1 << ", " << imm;
            if (rd != 0) registradores[rd] = registradores[rs1] & imm;
            break;
        case 0x1: // SLLI
        {
            uint32_t shamt = imm & 0x1F; // shamt são os 5 bits de baixo do imediato
            log_ss << "Executando SLLI x" << std::dec << rd << ", x" << rs1 << ", " << shamt;
            if (rd != 0) registradores[rd] = registradores[rs1] << shamt;
            break;
        }
        case 0x5: // SRLI / SRAI
        {
            uint32_t shamt = imm & 0x1F;
            uint32_t funct7_special = imm >> 5; // bits 5-11 do imediato
            if (funct7_special == 0x00) {
                // SRLI
                log_ss << "Executando SRLI x" << std::dec << rd << ", x" << rs1 << ", " << shamt;
                if (rd != 0) registradores[rd] = registradores[rs1] >> shamt;
            } else if (funct7_special == 0x20) {
                // SRAI
                log_ss << "Executando SRAI x" << std::dec << rd << ", x" << rs1 << ", " << shamt;
                if (rd != 0) registradores[rd] = static_cast<int32_t>(registradores[rs1]) >> shamt;
            }
            break;
        }
        default:
            log_ss << "ERRO: Tipo-I com funct3 desconhecido: 0x" << std::hex << inst.funct3();
            break;
    }

    // Instruções normais avançam o PC em 4
    contador_programa += 4;
    return log_ss.str();
}

std::string Core::handle_branch(const Instruction &inst) {
    std::stringstream log_ss;

    int32_t offset = inst.imediato_tipo_B();
    uint32_t rs1 = inst.rs1();
    uint32_t rs2 = inst.rs2();

    bool deve_desviar = false;
    switch (inst.funct3()) {
        case 0x0: // BEQ
            log_ss << "Executando BEQ x" << std::dec << rs1 << ", x" << rs2 << ", " << offset;
            if (registradores[rs1] == registradores[rs2]) deve_desviar = true;
            break;
        case 0x1: // BNE
            log_ss << "Executando BNE x" << std::dec << rs1 << ", x" << rs2 << ", " << offset;
            if (registradores[rs1] != registradores[rs2]) deve_desviar = true;
            break;
        default:
            log_ss << "ERRO: Branch com funct3 desconhecido: 0x" << std::hex << inst.funct3();
            break;
    }

    // Desvios têm lógica de PC diferente
    if (deve_desviar) {
        contador_programa += offset;
    } else {
        contador_programa += 4;
    }
    return log_ss.str();
}

std::string Core::handle_op_reg(const Instruction &inst) {
    std::stringstream log_ss;

    // Decodificação limpa
    uint32_t rd = inst.rd();
    uint32_t rs1 = inst.rs1();
    uint32_t rs2 = inst.rs2();
    uint32_t funct3 = inst.funct3();
    uint32_t funct7 = inst.funct7();

    if (funct3 == 0x0 && funct7 == 0x00) {
        // ADD
        log_ss << "Executando ADD x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] + registradores[rs2];
        }
    } else if (funct3 == 0x0 && funct7 == 0x20) {
        // SUB
        log_ss << "Executando SUB x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] - registradores[rs2];
        }
    } else if (funct3 == 0x2 && funct7 == 0x00) {
        // SLT
        log_ss << "Executando SLT x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = (static_cast<int32_t>(registradores[rs1]) < static_cast<int32_t>(registradores[rs2]))
                                    ? 1
                                    : 0;
        }
    } else if (funct3 == 0x4 && funct7 == 0x00) {
        // XOR
        log_ss << "Executando XOR x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] ^ registradores[rs2];
        }
    } else if (funct3 == 0x6 && funct7 == 0x00) {
        // OR
        log_ss << "Executando OR x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] | registradores[rs2];
        }
    } else if (funct3 == 0x7 && funct7 == 0x00) {
        // AND
        log_ss << "Executando AND x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] & registradores[rs2];
        }
    } else if (funct3 == 0x1 && funct7 == 0x00) {
        // SLL
        uint32_t shamt = registradores[rs2] & 0x1F; // Usa os 5 bits de baixo de rs2
        log_ss << "Executando SLL x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] << shamt;
        }
    } else if (funct3 == 0x5 && funct7 == 0x00) {
        // SRL
        uint32_t shamt = registradores[rs2] & 0x1F;
        log_ss << "Executando SRL x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = registradores[rs1] >> shamt;
        }
    } else if (funct3 == 0x5 && funct7 == 0x20) {
        // SRA
        uint32_t shamt = registradores[rs2] & 0x1F;
        log_ss << "Executando SRA x" << std::dec << rd << ", x" << rs1 << ", x" << rs2;
        if (rd != 0) {
            registradores[rd] = static_cast<int32_t>(registradores[rs1]) >> shamt;
        }
    } else {
        log_ss << "ERRO: Tipo-R com funct3/funct7 desconhecido!";
    }

    contador_programa += 4;
    return log_ss.str();
}

/**
 * @brief (Opcode 0x03) Trata instruções Tipo-I (Load).
 * Ex: LW, LB, LH, LBU, LHU
 */
std::string Core::handle_load(const Instruction &inst) {
    std::stringstream log_ss;

    // Decodificação limpa
    uint32_t rd = inst.rd();
    uint32_t rs1 = inst.rs1();
    int32_t imm = inst.imediato_tipo_I();
    uint32_t endereco = registradores[rs1] + imm;

    switch (inst.funct3()) {
        case 0x2: // LW (Load Word)
            log_ss << "Executando LW x" << std::dec << rd << ", " << imm << "(x" << rs1 << ")"
                    << " -> Endereco: 0x" << std::hex << endereco;
            if (rd != 0) {
                // Usa o cache para ler
                registradores[rd] = cache->lerDados(endereco);
            }
            break;
        default:
            log_ss << "ERRO: Load com funct3 desconhecido: 0x" << std::hex << inst.funct3();
            break;
    }

    contador_programa += 4;
    return log_ss.str();
}

/**
 * @brief (Opcode 0x23) Trata instruções Tipo-S (Store).
 * Ex: SW, SB, SH
 */
std::string Core::handle_store(const Instruction &inst) {
    std::stringstream log_ss;

    // Decodificação limpa
    uint32_t rs1 = inst.rs1();
    uint32_t rs2 = inst.rs2();
    int32_t imm = inst.imediato_tipo_S();
    auto endereco = static_cast<uint32_t>(static_cast<int32_t>(registradores[rs1]) + imm);

    switch (inst.funct3()) {
        case 0x2: {
            log_ss << "Executando SW x" << std::dec << rs2 << ", " << imm << "(x" << rs1 << ")"
                    << " -> Endereco: 0x" << std::hex << endereco;

            uint32_t valor = registradores[rs2]; // Agora esta inicialização é segura
            cache->escreverDados(endereco, valor);

            break;
        }
        default:
            log_ss << "ERRO: Store com funct3 desconhecido: 0x" << std::hex << inst.funct3();
            break;
    }

    contador_programa += 4;
    return log_ss.str();
}

/**
 * @brief (Opcode 0x37) Trata instrução LUI (Load Upper Immediate).
 */
std::string Core::handle_lui(const Instruction &inst) {
    std::stringstream log_ss;

    // Decodificação limpa
    uint32_t rd = inst.rd();
    int32_t imm = inst.imediato_tipo_U(); // Imediato Tipo-U!

    log_ss << "Executando LUI x" << std::dec << rd << ", 0x" << std::hex << (imm >> 12);

    if (rd != 0) {
        registradores[rd] = imm;
    }

    contador_programa += 4;
    return log_ss.str();
}

/**
 * @brief (Opcode 0x6F) Trata instrução JAL (Jump and Link).
 */
std::string Core::handle_jal(const Instruction &inst) {
    std::stringstream log_ss;

    // Decodificação limpa
    uint32_t rd = inst.rd();
    int32_t offset = inst.imediato_tipo_J(); // Imediato Tipo-J!

    log_ss << "Executando JAL x" << std::dec << rd << ", " << offset;

    // Salva o endereço da PRÓXIMA instrução (PC+4) em rd
    if (rd != 0) {
        registradores[rd] = contador_programa + 4;
    }

    // Atualiza o PC com o pulo (offset)
    contador_programa += offset;

    // Nota: O PC não é incrementado por 4 aqui! O 'offset' é o novo PC.
    return log_ss.str();
}
