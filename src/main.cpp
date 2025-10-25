#include "core/Core.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cstdint> // Para uint32_t
#include <QApplication>

#include "gui/launcherwindow.h"
#include "gui/mainwindow.h"

// --- FUNÇÕES HELPER PARA O MENU ---

// Valida a entrada de um número
int lerNumero() {
    int num;
    while (!(std::cin >> num)) {
        std::cout << "[ERRO] Entrada invalida. Por favor, insira um numero." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Tente novamente: ";
    }
    // Limpa o buffer de entrada para a próxima leitura (ex: getline)
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return num;
}

// Pede ao usuário um índice de registrador (ex: 5 para x5)
uint32_t lerRegistrador(const std::string &nome_reg) {
    while (true) {
        std::cout << "  Digite o registrador " << nome_reg << " (0-31): ";
        int reg = lerNumero();
        if (reg >= 0 && reg <= 31) {
            return static_cast<uint32_t>(reg);
        }
        std::cout << "[ERRO] Registrador invalido. Deve ser entre 0 e 31." << std::endl;
    }
}

// Pede ao usuário um valor imediato
int32_t lerImediato() {
    while (true) {
        std::cout << "  Digite o valor imediato (entre -2048 e 2047): ";
        int32_t imm = lerNumero();
        if (imm >= -2048 && imm <= 2047) {
            return imm;
        }
        std::cout << "[ERRO] Valor imediato invalido. Deve ser de 12 bits." << std::endl;
    }
}

uint32_t montar_tipo_R(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t montar_tipo_I(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
    return (static_cast<uint32_t>(imm) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

void executarInstrucaoDemo(Core &core) {
    std::cout << "\n--- Menu de Demonstracao de Instrucoes ---" << std::endl;
    std::cout << "Este modo permite testar uma instrucao isoladamente." << std::endl;
    std::cout << "Voce podera definir os valores iniciais dos registradores." << std::endl << std::endl;
    std::cout << "Qual instrucao deseja demonstrar?" << std::endl;
    std::cout << "  1. ADD (Soma: x_rd = x_rs1 + x_rs2)" << std::endl;
    std::cout << "  2. SUB (Subtracao: x_rd = x_rs1 - x_rs2)" << std::endl;
    std::cout << "  3. ADDI (Soma com Imediato: x_rd = x_rs1 + valor_fixo)" << std::endl;
    std::cout << "  4. ANDI (E Logico com Imediato: x_rd = x_rs1 & valor_fixo)" << std::endl;
    std::cout << "  5. ORI (OU Logico com Imediato: x_rd = x_rs1 | valor_fixo)" << std::endl;
    std::cout << "  0. Voltar" << std::endl;

    int escolha = lerNumero();
    if (escolha == 0) return;

    uint32_t rd = 0, rs1 = 0, rs2 = 0;
    int32_t imm = 0;
    uint32_t instrucao_codificada = 0;

    const uint32_t OPCODE_R = 0x33;
    const uint32_t OPCODE_I = 0x13;

    // Coletar operandos
    switch (escolha) {
        case 1: // ADD (R-Type)
        case 2: // SUB (R-Type)
            rd = lerRegistrador("rd (destino)");
            rs1 = lerRegistrador("rs1 (fonte 1)");
            rs2 = lerRegistrador("rs2 (fonte 2)");
            break;
        case 3: // ADDI (I-Type)
        case 4: // ANDI (I-Type)
        case 5: // ORI (I-Type)
            rd = lerRegistrador("rd (destino)");
            rs1 = lerRegistrador("rs1 (fonte 1)");
            imm = lerImediato(); // AGORA USA A FUNÇÃO CORRIGIDA
            break;
        default:
            std::cout << "[ERRO] Opcao invalida." << std::endl;
            return;
    }

    // Preparar o Core
    core.reset();

    // Definir valores nos registradores fonte
    if (escolha <= 2) {
        // Tipo R
        std::cout << "-> Preparando registradores..." << std::endl;
        std::cout << "  Qual valor inicial para x" << rs1 << "? ";
        core.set_register(rs1, lerNumero());
        std::cout << "  Qual valor inicial para x" << rs2 << "? ";
        core.set_register(rs2, lerNumero());
    } else {
        // Tipo I
        std::cout << "-> Preparando registradores..." << std::endl;
        std::cout << "  Qual valor inicial para x" << rs1 << "? ";
        core.set_register(rs1, lerNumero());
    }

    std::cout << "\n--- Estado ANTES da Execucao ---" << std::endl;

    // Montar a instrução
    // (A função montar_tipo_I agora vai funcionar, pois o 'imm' está limitado)
    switch (escolha) {
        case 1: // ADD (funct7=0x00, funct3=0x0)
            instrucao_codificada = montar_tipo_R(0x00, rs2, rs1, 0x0, rd, OPCODE_R);
            break;
        case 2: // SUB (funct7=0x20, funct3=0x0)
            instrucao_codificada = montar_tipo_R(0x20, rs2, rs1, 0x0, rd, OPCODE_R);
            break;
        case 3: // ADDI (funct3=0x0)
            instrucao_codificada = montar_tipo_I(imm, rs1, 0x0, rd, OPCODE_I);
            break;
        case 4: // ANDI (funct3=0x7)
            instrucao_codificada = montar_tipo_I(imm, rs1, 0x7, rd, OPCODE_I);
            break;
        case 5: // ORI (funct3=0x6)
            instrucao_codificada = montar_tipo_I(imm, rs1, 0x6, rd, OPCODE_I);
            break;
    }

    // Carregar e executar
    std::vector<uint32_t> programa_demo = {instrucao_codificada, 0x00000000};
    core.load_program(programa_demo);
    core.step();

    std::cout << "\n--- Estado APOS a Execucao ---" << std::endl;

    std::cout << std::dec;

    std::cout << "Pressione Enter para continuar... \n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


void modoSimulador(Core &core) {
    std::vector<uint32_t> programa_exemplo = {
        0xFB000313, // addi x6, x0, -80
        0x00435393, // slti x7, x6, 4
        0x40435413, // xori x8, x6, 1028
        0x00000000 // Instrução Nula (para finalizar)
    };
    bool programa_carregado = false;
    int escolha = -1;

    while (escolha != 0) {
        std::cout << "\n--- Modo Simulador de Programa ---" << std::endl;
        std::cout << "1. Carregar Programa Exemplo" << std::endl;
        std::cout << "2. Executar Programa Completo (Run)" << std::endl;
        std::cout << "3. Executar Proxima Instrucao (Step)" << std::endl;
        std::cout << "4. Exibir Registradores e PC" << std::endl;
        std::cout << "5. Resetar Core" << std::endl;
        std::cout << "0. Voltar ao Menu Principal" << std::endl;
        std::cout << "Escolha uma opcao: ";

        escolha = lerNumero();
        std::cout << std::endl;

        switch (escolha) {
            case 1:
                core.reset();
                core.load_program(programa_exemplo);
                programa_carregado = true;
                std::cout << "[INFO] Programa exemplo carregado e core resetado." << std::endl;
                break;
            case 2:
                if (!programa_carregado) {
                    std::cout << "[AVISO] Nenhum programa carregado." << std::endl;
                } else if (core.is_finished()) {
                    std::cout << "[AVISO] Simulacao ja finalizada. Resete (5) e carregue (1)." << std::endl;
                }
                break;
            case 3:
                if (!programa_carregado) {
                    std::cout << "[AVISO] Nenhum programa carregado." << std::endl;
                } else if (core.is_finished()) {
                    std::cout << "[INFO] Simulacao finalizada. Nao ha mais instrucoes." << std::endl;
                } else {
                    std::cout << "Executando (step)..." << std::endl;
                    core.step();
                }
                break;
            case 4:
                break;
            case 5:
                core.reset();
                programa_carregado = false;
                std::cout << "[INFO] Core resetado (PC e Registradores zerados)." << std::endl;
                break;
            case 0:
                break; // Sai do loop
            default:
                std::cout << "[ERRO] Opcao invalida." << std::endl;
                break;
        }
    }
}

// --- FUNÇÃO PARA O MENU DE AJUDA ---

void mostrarAjuda() {
    int escolha = -1;
    while (escolha != 0) {
        std::cout << "\n--- Menu de Ajuda ---" << std::endl;
        std::cout << "Sobre qual funcionalidade voce gostaria de saber mais?" << std::endl;
        std::cout << "  1. Modo Simulador" << std::endl;
        std::cout << "  2. Modo Demonstrador de Instrucoes" << std::endl;
        std::cout << "  0. Voltar ao Menu Principal" << std::endl;
        std::cout << "Escolha uma opcao: ";

        escolha = lerNumero();
        std::cout << std::endl;

        switch (escolha) {
            case 1:
                std::cout << "--- O que e o Modo Simulador? ---" << std::endl;
                std::cout << "Este modo carrega um programa de exemplo pre-definido na memoria do simulador.\n"
                        << "Voce pode executa-lo de duas formas:\n"
                        << " - Run (Executar Completo): O simulador executa todas as instrucoes do inicio ao fim, sem parar.\n"
                        << " - Step (Passo a Passo): O simulador executa apenas UMA instrucao e para, permitindo que voce\n"
                        << "   analise o estado dos registradores apos cada passo.\n"
                        << "Use esta opcao para entender como um programa completo altera o estado do processador." <<
                        std::endl;
                break;
            case 2:
                std::cout << "--- O que e o Modo Demonstrador? ---" << std::endl;
                std::cout << "Este modo funciona como um laboratorio para testar instrucoes individualmente.\n"
                        << "Voce escolhe uma instrucao (como ADD ou ADDI), e o programa te ajuda a monta-la,\n"
                        << "pedindo os registradores e valores necessarios.\n"
                        << "Antes da execucao, voce define os valores iniciais nos registradores de origem.\n"
                        << "O simulador entao executa APENAS essa instrucao e mostra o resultado.\n"
                        << "Use esta opcao para aprender exatamente o que cada instrucao faz." << std::endl;

                std::cout << "\nAs operacoes disponiveis para demonstracao sao:" << std::endl;
                std::cout << "\n- ADD (Soma): Realiza a SOMA do valor de dois registradores (rs1 + rs2)." << std::endl;
                std::cout << "- SUB (Subtracao): Realiza a SUBTRACAO do valor de dois registradores (rs1 - rs2)." <<
                        std::endl;
                std::cout <<
                        "- ADDI (Soma com Imediato): SOMA o valor de um registrador (rs1) com um numero fixo (o 'valor_fixo')."
                        << std::endl;
                std::cout <<
                        "- ANDI (E Logico com Imediato): Realiza a operacao logica 'AND' bit a bit entre um registrador e um numero fixo."
                        << std::endl;
                std::cout <<
                        "- ORI (OU Logico com Imediato): Realiza a operacao logica 'OR' bit a bit entre um registrador e um numero fixo."
                        << std::endl;
                break;
        }

        if (escolha != 0) {
            std::cout << "\nPressione Enter para continuar... \n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LauncherWindow w; // <--- Alterado (Inicia o Launcher)
    w.show();
    return a.exec();
}