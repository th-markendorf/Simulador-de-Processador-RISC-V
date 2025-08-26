//
// Created by rafael on 8/18/25.
//

#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H

#include <cstdint>
#include <vector>

class Core {
public:
    Core(size_t tamanho_memoria);
    void reset();
    void imprimir_register();
    void load_program(const std::vector<uint32_t>& programa);
    void run();

private:
    uint32_t fetch();
    void execute(uint32_t instrucao);

    uint32_t registradores[32];
    uint32_t contador_programa;
    std::vector<uint8_t> memoria;
};


#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H