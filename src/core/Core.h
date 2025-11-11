// src/core/Core.h

#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H

#include <vector>
#include <memory>
#include <array>
#include <string>

#include "RegistradoresPipeline.h"
#include "../cache/Cache.h"

class Core {
public:
    explicit Core(size_t tamanho_memoria);

    void reset();

    void load_program(const std::vector<uint32_t> &programa);

    void tick_clock(); // A função principal do pipeline

    // Funções de Interface/GUI (o que a GUI pode chamar)
    std::array<uint32_t, 32> get_registradores() const;

    uint32_t get_program_counter() const;

    bool is_finished() const;

    std::string set_register(int reg_index, uint32_t valor);

    uint8_t get_byte_memoria(uint32_t endereco) const;

private:
    // --- Membros Principais ---
    uint32_t registradores[32];
    uint32_t contador_programa;
    std::vector<uint8_t> memoria;
    std::unique_ptr<Cache> cache;
    bool m_do_flush = false;
    uint32_t m_new_pc_target = 0;

    // --- Membros do Pipeline ---
    bool m_pipeline_stall = false; // Sinal de "freio" para o IF e ID

    // 4 diferentes registradores do sistema de pipeline
    IF_ID_Register reg_if_id;
    ID_EX_Register reg_id_ex;
    EX_MEM_Register reg_ex_mem;
    MEM_WB_Register reg_mem_wb;

    // 5 funções do sistema de pipelines
    void pipeline_estagio_IF();

    void pipeline_estagio_ID();

    void pipeline_estagio_EX();

    void pipeline_estagio_MEM();

    void pipeline_estagio_WB();
};

#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H
