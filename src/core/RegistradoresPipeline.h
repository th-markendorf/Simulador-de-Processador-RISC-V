#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_REGISTRADORESPIPELINE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_REGISTRADORESPIPELINE_H
#pragma once
#include "Core.h"
#include "TiposPipeline.h"

struct IF_ID_Register {
    uint32_t instrucao; // A instrução buscada
    uint32_t pc; // O PC desta instrução
    bool valido = false; // Flag para controlar bolhas (stalls)
};

struct ID_EX_Register {
    // Sinais de Controle (decididos no estágio ID)
    uint32_t opcode; // <-- ADICIONADO: O opcode da instrução
    bool usar_ula = false;
    uint32_t rs1_index; // Índice do registrador fonte 1 (0-31)
    uint32_t rs2_index; // Índice do registrador fonte 2 (0-31)
    bool ler_memoria = false;
    bool escrever_memoria = false;
    bool escrever_registrador = false;
    OperacoesULA operacao_ula; // Usa o Enum
    bool mem_para_reg = false;

    // Dados (lidos do banco de registradores)
    int32_t valor_rs1;
    int32_t valor_rs2;
    int32_t imediato;
    uint32_t rd_destino; // Onde escrever o resultado
    uint32_t pc;
    bool valido = false;
    bool eh_branch = false; // É uma instrução BEQ, BNE, etc.
    bool eh_jal = false;    // É uma instrução JAL
    bool eh_jalr = false;
    bool eh_auipc = false;
    uint32_t funct3_branch; // Salva o tipo de branch (BEQ=0, BNE=1, etc.)
};

struct EX_MEM_Register {
    // Sinais de Controle (passados adiante)
    bool ler_memoria = false;
    bool escrever_memoria = false;
    bool escrever_registrador = false;
    bool mem_para_reg = false;

    // Dados (calculados pela ULA)
    int32_t resultado_ula;
    int32_t valor_para_escrever_memoria; // (é o valor_rs2)
    uint32_t rd_destino;
    bool valido = false;
};

struct MEM_WB_Register {
    // Sinais de Controle (passados adiante)
    bool escrever_registrador = false;
    bool mem_para_reg = false; // <-- ADICIONADO

    // Dados (o resultado final)
    int32_t dado_lido_da_memoria;
    int32_t resultado_ula;
    uint32_t rd_destino;
    bool valido = false;
};

#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_REGISTRADORESPIPELINE_H