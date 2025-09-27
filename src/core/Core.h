// src/core/Core.h

#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H

#include <vector>
#include <memory> // Adicionado para std::unique_ptr
#include "../cache/Cache.h" // Adicionado para a classe Cache

class Core {
public:
    explicit Core(size_t tamanho_memoria);
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

    // ponteiro para o cache
    std::unique_ptr<Cache> cache;
};

#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H