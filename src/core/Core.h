// src/core/Core.h

#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H

#include <vector>
#include <memory>

#include "Instruction.h"
#include "../cache/Cache.h"

class Core {
public:
    explicit Core(size_t tamanho_memoria);
    void reset();
    std::array<uint32_t, 32> get_registradores() const;
    void load_program(const std::vector<uint32_t>& programa);
    std::string step();
    uint32_t get_program_counter() const;
    bool is_finished() const;
    std::string set_register(int reg_index, uint32_t valor);
    uint8_t get_byte_memoria(uint32_t endereco) const;

private:
    uint32_t fetch();
    std::string execute(uint32_t instrucao);

    std::string handle_op_imm(const Instruction& inst);  // 0x13
    std::string handle_op_reg(const Instruction& inst);  // 0x33
    std::string handle_load(const Instruction& inst);    // 0x03
    std::string handle_store(const Instruction& inst);   // 0x23
    std::string handle_branch(const Instruction& inst); // 0x63
    std::string handle_lui(const Instruction& inst);      // 0x37
    std::string handle_jal(const Instruction& inst);      // 0x6F

    uint32_t registradores[32];
    uint32_t contador_programa;
    std::vector<uint8_t> memoria;

    // ponteiro para o cache
    std::unique_ptr<Cache> cache;
};

#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_CORE_H