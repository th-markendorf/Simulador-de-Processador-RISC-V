//
// Created by rafael on 8/18/25.
//

#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H

#include <cstdint>

class Core {
public:
    Core();
    void reset();
    void imprimir_register();

private:
    uint8_t registradores[32];
    uint32_t contador_programa;
};


#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H