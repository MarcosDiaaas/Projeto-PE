#ifndef COMPRESSAO_H
#define COMPRESSAO_H

#include <stdio.h>
#include "dicionario.h"      /* Dicionario e Codigo */

/* =====================================================================
 *  ETAPA 4 - CODIFICAÇÃO (COMPRESSÃO) - Huffman por byte (0..255)
 * =====================================================================
 *
 *  ETAPAS DO PROGRAMA
 *    Etapa 1 (frequências) .... calcular_frequencias()   huffman.c
 *    Etapa 2 (árvore) ......... arvore_construir()       arvore.c
 *    Etapa 3 (códigos) ........ dicionario_gerar()       dicionario.c
 *    Etapa 4 (esta) ........... comprimir_arquivo()      compressao.c
 *  O comprimir_arquivo(caminho_entrada, caminho_saida) chama as quatro,
 *  nessa ordem, e retorna 0 se deu certo.
 *
 *  FORMATO DO ARQUIVO COMPRIMIDO (a descompressão lê exatamente isto)
 *  Inteiros em big-endian (byte mais significativo primeiro).
 *
 *    [4 bytes]  assinatura 'Z' 'I' 'P' 'Z'
 *    [4 bytes]  tamanho do arquivo original, em bytes
 *    [2 bytes]  n = nº de símbolos (bytes) distintos, de 0 a 256
 *    n vezes:   [1 byte] símbolo  +  [4 bytes] frequência
 *    [resto]    fluxo de bits: código de cada byte do original, na
 *               ordem do arquivo, bit mais significativo primeiro; o
 *               último byte é completado com zeros.
 *
 *  PARA A DESCOMPRESSÃO: leia o cabeçalho, monte o vetor de
 *  frequências (freq[símbolo] = frequência) e chame arvore_construir()
 *  nele. Sai a MESMA árvore da compressão (o desempate da fila é fixo).
 *  Depois leia os bits: 0 = filho esquerdo, 1 = filho direito; ao chegar
 *  numa folha, grave o símbolo e volte à raiz. Pare ao decodificar
 *  "tamanho original" símbolos (os zeros do último byte são ignorados).
 *  Arquivo vazio: n = 0 e nenhum bit. Um único símbolo: código "0".
 * ===================================================================== */

/* ---- Escritor de bits (o C só grava bytes inteiros) ---- */
typedef struct EscritorBits {
    FILE *arq;
    unsigned char buffer;      /* bits acumulados, alinhados à direita */
    int nBits;                 /* quantos bits há no buffer (0..7) */
    unsigned long bytes_gravados;
} EscritorBits;

void escritor_inicia(EscritorBits *E, FILE *arq);
void escritor_bit(EscritorBits *E, int bit);
void escritor_codigo(EscritorBits *E, const Codigo *c);  /* código do dicionário */
void escritor_finaliza(EscritorBits *E);                 /* chamar UMA vez, no fim */

/* ---- Cabeçalho e codificação ---- */
int gravar_cabecalho(FILE *saida, const unsigned long freq[NUM_SIMBOLOS],
                     unsigned long total_bytes);
int codificar_arquivo(FILE *entrada, FILE *saida, const Dicionario *dic,
                      unsigned long total_esperado);

/* ---- Utilitários ---- */
long tamanho_arquivo(FILE *f);
void exibir_estatisticas(const char *nome_original, const char *nome_comprimido,
                         long tam_original, long tam_comprimido, double segundos);

/* ---- Compressão completa (etapas 1 a 4). Retorna 0 se OK. ----
 * Não altera o arquivo original: sempre cria um arquivo novo. */
int comprimir_arquivo(const char *caminho_entrada, const char *caminho_saida);

#endif
