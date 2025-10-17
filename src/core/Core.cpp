#include "Core.h"

#include <iomanip>
#include <iostream>
#include <ostream>

Core::Core(size_t tamanho_memoria) : memoria(tamanho_memoria, 0) {
    cache = std::make_unique<Cache>(4096, 16, memoria);
    reset();
}

void Core::reset() {
    contador_programa = 0x0;
    for (int i = 0; i < 32; ++i) {
        registradores[i] = 0;
    }
}

bool Core::is_finished() {
    return contador_programa >= memoria.size();
}

void Core::step() {
    if (is_finished()) {
        std::cout << "Simulacao ja finalizada. PC em 0x" << std::hex << contador_programa << std::endl;
        return;
    }

    uint32_t instrucao = fetch();

    execute(instrucao);
}

void Core::imprimir_register() {
    std::cout << "Registers:" << std::endl;
    for (int i = 0; i < 32; i += 1) {
        std::cout << std::setw(3) << std::right << "x" << i << ": "
                  << "0x" << std::hex << std::setw(8) << std::setfill('0') << registradores[i];

        // adiciona o valor em decimal quando nao for zero
        if (registradores[i] != 0) {
            std::cout << "  (Decimal: " << std::dec << registradores[i] << ")";
        }

        std::cout << std::endl;
    }
    std::cout << std::setw(3) << std::right << "prog counter" << ": "
            << "0x" << std::hex << std::setw(8) << std::setfill('0') << contador_programa << std::endl;
    std::cout << "--------------------------------" << std::endl;
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

void Core::execute(uint32_t instrucao) {
    if (instrucao == 0) {
        contador_programa = memoria.size();
        std::cout << "Instrucao nula, finalizando." << std::endl;
        return;
    }

    uint32_t opcode = instrucao & 0x7F;

    switch (opcode) {
        case 0x13: {
            uint32_t rd = (instrucao >> 7) & 0x1F;
            uint32_t funct3 = (instrucao >> 12) & 0x7;
            uint32_t rs1 = (instrucao >> 15) & 0x1F;
            int32_t imm = static_cast<int32_t>(instrucao) >> 20;

            if (funct3 == 0x0) {
                std::cout << "Executando ADDI x" << std::dec << rd << ", x" << rs1 << ", " << imm << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] + imm;
                }
            } else if (funct3 == 0x2) {
                std::cout << "Executando SLTI x" << std::dec << rd << ", x" << rs1 << ", " << imm << std::endl;
                if (rd != 0) {
                    registradores[rd] = (static_cast<int32_t>(registradores[rs1]) < imm) ? 1 : 0;
                }
            } else if (funct3 == 0x4) {
                std::cout << "Executando XORI x" << std::dec << rd << ", x" << rs1 << ", " << imm << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] ^ imm;
                }
            } else if (funct3 == 0x6) {
                std::cout << "Executando ORI x" << std::dec << rd << ", x" << rs1 << ", " << imm << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] | imm;
                }
            } else if (funct3 == 0x7) {
                std::cout << "Executando ANDI x" << std::dec << rd << ", x" << rs1 << ", " << imm << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] & imm;
                }
            } else if (funct3 == 0x1) {
                uint32_t shamt = imm & 0x1F;
                std::cout << "Executando SLLI x" << std::dec << rd << ", x" << rs1 << ", " << shamt << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] << shamt;
                }
            } else if (funct3 == 0x5) {
                uint32_t shamt = imm & 0x1F;
                uint32_t funct7_special = imm >> 5;

                if (funct7_special == 0x00) {
                    std::cout << "Executando SRLI x" << std::dec << rd << ", x" << rs1 << ", " << shamt << std::endl;
                    if (rd != 0) {
                        registradores[rd] = registradores[rs1] >> shamt;
                    }
                } else if (funct7_special == 0x20) {
                    std::cout << "Executando SRAI x" << std::dec << rd << ", x" << rs1 << ", " << shamt << std::endl;
                    if (rd != 0) {
                        registradores[rd] = static_cast<int32_t>(registradores[rs1]) >> shamt;
                    }
                }
            } else {
                std::cerr << "Tipo-I com funct3 desconhecido: 0x" << std::hex << funct3 << std::endl;
            }

            contador_programa += 4;
            break;
        }
        case 0x33: {
            uint32_t rd = (instrucao >> 7) & 0x1F;
            uint32_t funct3 = (instrucao >> 12) & 0x7;
            uint32_t rs1 = (instrucao >> 15) & 0x1F;
            uint32_t rs2 = (instrucao >> 20) & 0x1F;
            uint32_t funct7 = (instrucao >> 25) & 0x7F;

            if (funct3 == 0x0 && funct7 == 0x00) {
                std::cout << "Executando ADD x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] + registradores[rs2];
                }
            } else if (funct3 == 0x0 && funct7 == 0x20) {
                std::cout << "Executando SUB x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] - registradores[rs2];
                }
            } else if (funct3 == 0x2 && funct7 == 0x00) {
                std::cout << "Executando SLT x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = (static_cast<int32_t>(registradores[rs1]) < static_cast<int32_t>(registradores[
                                             rs2]))
                                            ? 1
                                            : 0;
                }
            } else if (funct3 == 0x4 && funct7 == 0x00) {
                std::cout << "Executando XOR x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] ^ registradores[rs2];
                }
            } else if (funct3 == 0x6 && funct7 == 0x00) {
                std::cout << "Executando OR x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] | registradores[rs2];
                }
            } else if (funct3 == 0x7 && funct7 == 0x00) {
                std::cout << "Executando AND x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] & registradores[rs2];
                }
            } else if (funct3 == 0x1 && funct7 == 0x00) {
                uint32_t shamt = registradores[rs2] & 0x1F; // Usa os 5 bits de baixo de rs2
                std::cout << "Executando SLL x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] << shamt;
                }
            } else if (funct3 == 0x5 && funct7 == 0x00) {
                uint32_t shamt = registradores[rs2] & 0x1F;
                std::cout << "Executando SRL x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = registradores[rs1] >> shamt;
                }
            } else if (funct3 == 0x5 && funct7 == 0x20) {
                uint32_t shamt = registradores[rs2] & 0x1F;
                std::cout << "Executando SRA x" << std::dec << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
                if (rd != 0) {
                    registradores[rd] = static_cast<int32_t>(registradores[rs1]) >> shamt;
                }
            } else {
                std::cerr << "Tipo-R com funct3/funct7 desconhecido!" << std::endl;
            }

            contador_programa += 4;
            break;
        }

        case 0x03: {
            uint32_t rd = (instrucao >> 7) & 0x1F;
            uint32_t funct3 = (instrucao >> 12) & 0x7;
            uint32_t rs1 = (instrucao >> 15) & 0x1F;
            int32_t imm = static_cast<int32_t>(instrucao) >> 20;

            if (funct3 == 0x2) {
                uint32_t endereco = registradores[rs1] + imm;
                std::cout << "Executando LW x" << std::dec << rd << ", " << imm << "(x" << rs1 << ")" <<
                        " -> Endereco: 0x" << std::hex << endereco << std::endl;

                if (rd != 0) {
                    registradores[rd] = cache->lerDados(endereco);
                }
            }
            contador_programa += 4;
            break;
        }

        case 0x23: {
            uint32_t funct3 = (instrucao >> 12) & 0x7;
            uint32_t rs1 = (instrucao >> 15) & 0x1F;
            uint32_t rs2 = (instrucao >> 20) & 0x1F;

            uint32_t imm_4_0 = (instrucao >> 7) & 0x1F;
            uint32_t imm_11_5 = (instrucao >> 25) & 0x7F;
            int32_t imm = static_cast<int32_t>((imm_11_5 << 5) | imm_4_0);
            if (imm & 0x800) {
                imm |= 0xFFFFF000;
            }

            if (funct3 == 0x2) {
                uint32_t endereco = registradores[rs1] + imm;
                std::cout << "Executando SW x" << std::dec << rs2 << ", " << imm << "(x" << rs1 << ")" <<
                        " -> Endereco: 0x" << std::hex << endereco << std::endl;

                uint32_t valor = registradores[rs2];
                cache->escreverDados(endereco, valor);
            }
            contador_programa += 4;
            break;
        }

        case 0x63: {
            uint32_t funct3 = (instrucao >> 12) & 0x7;
            uint32_t rs1 = (instrucao >> 15) & 0x1F;
            uint32_t rs2 = (instrucao >> 20) & 0x1F;

            uint32_t imm_12 = (instrucao >> 31) & 0x1;
            uint32_t imm_11 = (instrucao >> 7) & 0x1;
            uint32_t imm_10_5 = (instrucao >> 25) & 0x3F;
            uint32_t imm_4_1 = (instrucao >> 8) & 0xF;
            int32_t offset = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
            if (offset & 0x1000) {
                offset |= 0xFFFFE000;
            }

            bool deve_desviar = false;
            if (funct3 == 0x0) {
                std::cout << "Executando BEQ x" << std::dec << rs1 << ", x" << rs2 << ", " << offset << std::endl;
                if (registradores[rs1] == registradores[rs2]) {
                    deve_desviar = true;
                }
            } else if (funct3 == 0x1) {
                std::cout << "Executando BNE x" << std::dec << rs1 << ", x" << rs2 << ", " << offset << std::endl;
                if (registradores[rs1] != registradores[rs2]) {
                    deve_desviar = true;
                }
            } else {
                std::cerr << "Branch com funct3 desconhecido!" << std::endl;
            }

            if (deve_desviar) {
                contador_programa += offset;
            } else {
                contador_programa += 4;
            }
            break;
        }

        case 0x6F: {
            uint32_t rd = (instrucao >> 7) & 0x1F;

            uint32_t imm_20 = (instrucao >> 31) & 0x1;
            uint32_t imm_19_12 = (instrucao >> 12) & 0xFF;
            uint32_t imm_11 = (instrucao >> 20) & 0x1;
            uint32_t imm_10_1 = (instrucao >> 21) & 0x3FF;
            int32_t offset = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1);
            if (offset & 0x100000) {
                offset |= 0xFFF00000;
            }

            std::cout << "Executando JAL x" << std::dec << rd << ", " << offset << std::endl;

            if (rd != 0) {
                registradores[rd] = contador_programa + 4;
            }
            contador_programa += offset;
            break;
        }
        case 0x37: {
            uint32_t rd = (instrucao >> 7) & 0x1F;
            uint32_t imm = instrucao & 0xFFFFF000;

            std::cout << "Executando LUI x" << std::dec << rd << ", 0x" << std::hex << (imm >> 12) << std::endl;

            if (rd != 0) {
                registradores[rd] = imm;
            }

            contador_programa += 4;
            break;
        }

        default:
            std::cerr << "Opcode desconhecido: 0x" << std::hex << opcode << std::endl;
            contador_programa += 4;
            break;
    }
    registradores[0] = 0;
}

void Core::set_register(int reg_index, uint32_t valor) {
    if (reg_index > 0 && reg_index < 32) {
        registradores[reg_index] = valor;
    } else if (reg_index == 0) {
        std::cout << "[AVISO] Nao e permitido alterar o registrador x0 (zero)." << std::endl;
    } else {
        std::cerr << "[ERRO] Tentativa de acessar registrador invalido: " << reg_index << std::endl;
    }
}

void Core::run() {
    std::cout << "Iniciando a simulacao (modo run)..." << std::endl;

    while (!is_finished()) {
        uint32_t instrucao_atual = fetch();
        execute(instrucao_atual);

        if (instrucao_atual == 0) {
            break;
        }
    }

    std::cout << "Simulacao finalizada." << std::endl;
    imprimir_register();
}
