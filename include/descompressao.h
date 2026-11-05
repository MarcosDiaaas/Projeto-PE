#ifndef DESCOMPRESSAO_H
#define DESCOMPRESSAO_H

#include <stdio.h>
#include "arvore.h"          /* ArvoreHuffman, No, NUM_SIMBOLOS */

/* =====================================================================
 *  ETAPA 5 - DECODIFICAÇÃO (DESCOMPRESSÃO)
 * =====================================================================
 *
 *  Faz o caminho inverso da Etapa 4 (compressao.c). Lê um arquivo no
 *  formato ZIPZ (descrito em compressao.h) e recria o arquivo original,
 *  byte a byte.
 *
 *  PASSO A PASSO (descomprimir_arquivo)
 *    1. Lê o cabeçalho: assinatura "ZIPZ", tamanho original e a tabela
 *       (símbolo, frequência).
 *    2. Monta o vetor de frequências e chama arvore_construir(). Como o
 *       desempate da fila de prioridade é fixo, sai a MESMA árvore que a
 *       compressão usou.
 *    3. Lê os bits do resto do arquivo, um por um, andando na árvore:
 *       0 = filho esquerdo, 1 = filho direito. Ao chegar numa folha,
 *       grava o símbolo dela e volta para a raiz.
 *    4. Para depois de gravar "tamanho original" bytes. Assim os zeros
 *       que completaram o último byte são ignorados.
 *
 *  O cabeçalho é conferido (assinatura, símbolos repetidos, soma das
 *  frequências = tamanho original). Arquivo corrompido gera erro em vez
 *  de um arquivo de saída errado.
 * ===================================================================== */

/* ---- Leitor de bits (o par do EscritorBits da compressão) ---- */
typedef struct LeitorBits {
    FILE *arq;
    unsigned char buffer;      /* byte atual sendo lido */
    int nBits;                 /* quantos bits do buffer ainda faltam ler (0..8) */
} LeitorBits;

void leitor_inicia(LeitorBits *L, FILE *arq);
/* Retorna o próximo bit (0 ou 1), ou -1 se o arquivo acabou. */
int leitor_bit(LeitorBits *L);

/* ---- Cabeçalho e decodificação ---- */

/* Lê e valida o cabeçalho. Preenche freq[] e *total_bytes.
 * Retorna 0 se OK, -1 se o arquivo não é ZIPZ ou está corrompido. */
int ler_cabecalho(FILE *entrada, unsigned long freq[NUM_SIMBOLOS],
                  unsigned long *total_bytes);

/* Lê os bits de 'entrada' (logo após o cabeçalho) e grava em 'saida'
 * os 'total_bytes' símbolos decodificados. Retorna 0 se OK. */
int decodificar_arquivo(FILE *entrada, FILE *saida, const ArvoreHuffman *arv,
                        unsigned long total_bytes);

/* ---- Descompressão completa. Retorna 0 se OK. ----
 * Não altera o arquivo comprimido: sempre cria um arquivo novo. */
int descomprimir_arquivo(const char *caminho_entrada, const char *caminho_saida);

#endif
