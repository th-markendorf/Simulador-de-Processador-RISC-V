#include "Cache.h"
#include <cmath>

Cache::Cache(uint32_t tamanho_cache, uint32_t tamanho_bloco, std::vector<uint8_t>& memoria_principal)
    : tamanho_cache(tamanho_cache),
      tamanho_bloco(tamanho_bloco),
      qtd_linhas(tamanho_cache / tamanho_bloco),
      memoria_principal(memoria_principal)
{
    linhas.reserve(qtd_linhas);

    for (uint32_t i = 0; i < qtd_linhas; ++i)
    {
        linhas.emplace_back(tamanho_bloco);
    }
}

uint32_t Cache::lerDados(uint32_t endereco)
{
    // Calcula o número de bits para o offset e para o índice
    // log2(n) nos dá o número de bits necessários para representar 'n' itens
    auto num_bits_offset = static_cast<uint32_t>(log2(tamanho_bloco));
    auto num_bits_indice = static_cast<uint32_t>(log2(qtd_linhas));

    // Extrai o índice do endereço
    // Ex: 0b...[TAG]...[ÍNDICE]...[OFFSET]
    // Para pegar o índice, desloca o endereço para a direita para remover o offset
    uint32_t indice = (endereco >> num_bits_offset) & (qtd_linhas - 1);

    // Extrai a tag do endereço
    uint32_t tag = endereco >> (num_bits_offset + num_bits_indice);

    LinhaCache& linha = linhas[indice];


    // Verifica se é um hit ou miss, se encontrou ou não o dado válido na cache com a tag correta
    if (linha.valida && linha.tag == tag)
    {
    }
    else
    {
        // usa operadores bitwise para encontrar o inicio do bloco
        uint32_t endereco_inicio_bloco = endereco & ~(tamanho_bloco - 1);

        for (uint32_t i = 0; i < tamanho_bloco; ++i)
        {
            linha.dados[i] = memoria_principal[endereco_inicio_bloco + i];
        }

        linha.tag = tag;
        linha.valida = true;
    }


    // Tanto em caso de hit quanto após tratar um miss, o dado agora está na 'linha.dados'.
    uint32_t offset = endereco & (tamanho_bloco - 1);

    uint32_t valor = 0;
    valor |= static_cast<uint32_t>(linha.dados[offset + 0]) << 0;
    valor |= static_cast<uint32_t>(linha.dados[offset + 1]) << 8;
    valor |= static_cast<uint32_t>(linha.dados[offset + 2]) << 16;
    valor |= static_cast<uint32_t>(linha.dados[offset + 3]) << 24;

    return valor;
}

void Cache::escreverDados(uint32_t endereco, uint32_t valor)
{
    // Política Write-Through: Escrever sempre na Memória Principal

    // Desmonta o valor de 32 bits em 4 bytes e os escreve na memória principal
    memoria_principal[endereco + 0] = (valor >> 0) & 0xFF;
    memoria_principal[endereco + 1] = (valor >> 8) & 0xFF;
    memoria_principal[endereco + 2] = (valor >> 16) & 0xFF;
    memoria_principal[endereco + 3] = (valor >> 24) & 0xFF;

    auto num_bits_offset = static_cast<uint32_t>(log2(tamanho_bloco));
    auto num_bits_indice = static_cast<uint32_t>(log2(qtd_linhas));

    uint32_t indice = (endereco >> num_bits_offset) & (qtd_linhas - 1);
    uint32_t tag = endereco >> (num_bits_offset + num_bits_indice);

    LinhaCache& linha = linhas[indice];


    // Apenas se for um HIT, também atualiza o valor no cache.
    if (linha.valida && linha.tag == tag)
    {
        // O bloco está no cache, então atualiza o valor aqui também.
        uint32_t offset = endereco & (tamanho_bloco - 1);

        linha.dados[offset + 0] = (valor >> 0) & 0xFF;
        linha.dados[offset + 1] = (valor >> 8) & 0xFF;
        linha.dados[offset + 2] = (valor >> 16) & 0xFF;
        linha.dados[offset + 3] = (valor >> 24) & 0xFF;
    }
    // Política No-Write-Allocate: Se o dado não está no cache nós NÃO o trazemos para o cache. Simplesmente não fazemos nada.
}
