//
// Created by rafael on 8/18/25.
//

#include "Core.h"

#include <iomanip>
#include <iostream>
#include <ostream>

Core::Core(size_t tamanho_memoria) : memoria(tamanho_memoria, 0) {
    reset();
}

void Core::reset() {
    contador_programa = 0x0;
    for (int i = 0; i < 32; ++i) {
        registradores[i] = 0;
    }
}

void Core::imprimir_register() {
    std::cout << "Registers:" << std::endl;
    for (int i = 0; i < 32; i += 1) {
        std::cout << std::setw(3) << std::right << "x" << i << ": "
                << "0x" << std::hex << std::setw(8) << std::setfill('0') << registradores[i] << "   " << std::endl;;
    }
    std::cout << std::setw(3) << std::right << "prog counter" << ": "
            << "0x" << std::hex << std::setw(8) << std::setfill('0') << contador_programa << std::endl;
    std::cout << "--------------------------------" << std::endl;
}

void Core::load_program(const std::vector<uint32_t>& programa) {
    for (size_t i = 0; i < programa.size(); ++i) {
        memoria[i * 4 + 0] = (programa[i] >> 0) & 0xFF;
        memoria[i * 4 + 1] = (programa[i] >> 8) & 0xFF;
        memoria[i * 4 + 2] = (programa[i] >> 16) & 0xFF;
        memoria[i * 4 + 3] = (programa[i] >> 24) & 0xFF;
    }
}

uint32_t Core::fetch() {
    uint32_t instrucao = 0;
    instrucao |= static_cast<uint32_t>(memoria[contador_programa + 0]) << 0;
    instrucao |= static_cast<uint32_t>(memoria[contador_programa + 1]) << 8;
    instrucao |= static_cast<uint32_t>(memoria[contador_programa + 2]) << 16;
    instrucao |= static_cast<uint32_t>(memoria[contador_programa + 3]) << 24;
    return instrucao;
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
            uint32_t rs1 = (instrucao >> 15) & 0x1F;
            int32_t imm = static_cast<int32_t>(instrucao) >> 20;

            std::cout << "Executando ADDI x" << rd << ", x" << rs1 << ", " << imm << std::endl;
            if (rd != 0) {
                registradores[rd] = registradores[rs1] + imm;
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

void Core::run() {
    std::cout << "Iniciando a simulacao..." << std::endl;
    while (contador_programa < memoria.size()) {
        uint32_t instrucao = fetch();
        execute(instrucao);
        imprimir_register();
    }
    std::cout << "Simulacao finalizada." << std::endl;
}
