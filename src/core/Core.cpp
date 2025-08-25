//
// Created by rafael on 8/18/25.
//

#include "Core.h"

#include <iomanip>
#include <iostream>
#include <ostream>

Core::Core() {
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
