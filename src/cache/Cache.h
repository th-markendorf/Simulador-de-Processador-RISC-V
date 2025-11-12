#ifndef SIMULADOR_DE_PROCESSADOR_RISC_V_CACHE_H
#define SIMULADOR_DE_PROCESSADOR_RISC_V_CACHE_H

#include <cstdint>
#include <vector>

class Cache
{
public:
    Cache(uint32_t tamanho_cache, uint32_t tamanho_bloco, std::vector<uint8_t>& memoria_principal);

    void reset();
    uint32_t lerDados(uint32_t endereco);
    void escreverDados(uint32_t endereco, uint32_t valor);

    struct LinhaCache
    {
        bool valida = false;
        uint32_t tag = 0;
        std::vector<uint8_t> dados;
        explicit LinhaCache(size_t tamanho_bloco) : dados(tamanho_bloco, 0){ }
    };

    const std::vector<LinhaCache>& get_linhas() const {
        return linhas;
    }

private:
    uint32_t tamanho_cache;
    uint32_t tamanho_bloco;
    uint32_t qtd_linhas;

    // Referência para a memória principal para buscar dados em caso de miss
    std::vector<uint8_t>& memoria_principal;

    // O vetor que armazena todas as linhas do cache
    std::vector<LinhaCache> linhas;
};

#endif //SIMULADOR_DE_PROCESSADOR_RISC_V_CACHE_H
