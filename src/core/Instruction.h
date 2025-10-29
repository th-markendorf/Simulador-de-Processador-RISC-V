#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_INSTRUCTION_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_INSTRUCTION_H
#include <cstdint>

/**
 * @struct Instruction
 * @brief Decodifica uma instrução RISC-V de 32 bits.
 */
struct Instruction {
    // A instrução bruta de 32 bits.
    uint32_t palavra_instrucao;

    explicit Instruction(uint32_t instrucao_em_32bits) : palavra_instrucao(instrucao_em_32bits) {
    }


    // opcode: O código da operação (define o formato da instrução).
    uint32_t opcode() const { return palavra_instrucao & 0x7F; }

    // rd: O registrador de destino (onde o resultado é escrito).
    uint32_t rd() const { return (palavra_instrucao >> 7) & 0x1F; }

    // funct3: Código de função de 3 bits (ajuda a definir a operação).
    uint32_t funct3() const { return (palavra_instrucao >> 12) & 0x7; }

    // rs1: O primeiro registrador fonte (primeiro operando).
    uint32_t rs1() const { return (palavra_instrucao >> 15) & 0x1F; }

    // rs2: O segundo registrador fonte (segundo operando).
    uint32_t rs2() const { return (palavra_instrucao >> 20) & 0x1F; }

    // funct7: Código de função de 7 bits (ajuda a definir a operação).
    uint32_t funct7() const { return (palavra_instrucao >> 25) & 0x7F; }


    // --- Imediatos (Valores Constantes) ---

    // Retorna o imediato de 12 bits do formato Tipo-I (usado por ADDI, LW).
    int32_t imediato_tipo_I() const {
        return static_cast<int32_t>(palavra_instrucao) >> 20;
    }

    // Remonta e retorna o imediato de 12 bits do formato Tipo-S (usado por SW).
    int32_t imediato_tipo_S() const {
        uint32_t imm_4_0 = (palavra_instrucao >> 7) & 0x1F;
        uint32_t imm_11_5 = (palavra_instrucao >> 25) & 0x7F;
        int32_t imm = static_cast<int32_t>((imm_11_5 << 5) | imm_4_0);
        return (imm & 0x800) ? (imm | 0xFFFFF000) : imm;
    }

    // Remonta e retorna o imediato de 13 bits do formato Tipo-B (usado por BEQ).
    int32_t imediato_tipo_B() const {
        uint32_t imm_12 = (palavra_instrucao >> 31) & 0x1;
        uint32_t imm_11 = (palavra_instrucao >> 7) & 0x1;
        uint32_t imm_10_5 = (palavra_instrucao >> 25) & 0x3F;
        uint32_t imm_4_1 = (palavra_instrucao >> 8) & 0xF;
        int32_t offset = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
        return (offset & 0x1000) ? (offset | 0xFFFFE000) : offset;
    }

    // Retorna o imediato de 20 bits do formato Tipo-U (usado por LUI).
    int32_t imediato_tipo_U() const {
        return static_cast<int32_t>(palavra_instrucao & 0xFFFFF000);
    }

    // Remonta e retorna o imediato de 21 bits do formato Tipo-J (usado por JAL).
    int32_t imediato_tipo_J() const {
        uint32_t imm_20 = (palavra_instrucao >> 31) & 0x1;
        uint32_t imm_19_12 = (palavra_instrucao >> 12) & 0xFF;
        uint32_t imm_11 = (palavra_instrucao >> 20) & 0x1;
        uint32_t imm_10_1 = (palavra_instrucao >> 21) & 0x3FF;
        int32_t offset = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1);
        return (offset & 0x100000) ? (offset | 0xFFF00000) : offset;
    }
};

#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_INSTRUCTION_H
