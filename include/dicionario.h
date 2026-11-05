/*
 * ============================================================================
 *  dicionario.h  -  TAD Dicionario (tabela simbolo -> codigo binario)
 * ============================================================================
 *  Responsavel: Pessoa 2 (Etapa 3 - Geracao de Codigos)
 *
 *  O QUE ESTE MODULO FAZ
 *  ---------------------
 *  Percorre a Arvore de Huffman da raiz ate cada folha. O caminho feito e o
 *  codigo do simbolo:  esquerda = 0,  direita = 1.
 *
 *  Exemplo ("ABRACADABRA"):     A -> 0      B -> 110    R -> 111
 *                               C -> 100    D -> 101
 *
 *  Por que os codigos sao PREFIXADOS (nenhum e o comeco de outro)?
 *  Porque os simbolos ficam somente nas FOLHAS: o caminho ate uma folha
 *  nunca passa por outra folha. Assim o descompressor le os bits e, ao
 *  chegar numa folha, sabe sem ambiguidade que o simbolo terminou.
 *
 *  DECISAO DE PROJETO: tabela de acesso direto
 *  -------------------------------------------
 *  O dicionario e um vetor indexado pelo proprio byte (0..255). Na
 *  compressao (Pessoa 3), achar o codigo de um byte custa O(1):
 *  dic->codigos[byte]. Nenhuma busca, nenhuma ordenacao.
 *
 *  Os bits de cada codigo ficam COMPACTADOS (8 bits por byte), prontos
 *  para a escrita bit a bit com << e | da Etapa 4.
 *
 *  COMO A PESSOA 3 USA (para cada byte b lido do arquivo original):
 *      for (i = 0; i < dicionario_tamanho(&dic, b); i++)
 *          escrever_bit(saida, dicionario_bit(&dic, b, i));
 * ============================================================================
 */
#ifndef DICIONARIO_H
#define DICIONARIO_H

#include <stdio.h>
#include "arvore.h"

/* Com 256 simbolos, o pior caso (arvore "degenerada", frequencias tipo
 * Fibonacci) tem altura 255: um codigo pode ter ate 255 bits. */
#define MAX_BITS_CODIGO 256
#define MAX_BYTES_CODIGO (MAX_BITS_CODIGO / 8)          /* = 32 bytes */

/*
 * Codigo de um simbolo.
 *   bits    : sequencia de bits compactada. O bit i esta no byte i/8,
 *             na posicao (7 - i%8)  ->  ordem "MSB primeiro".
 *   tamanho : quantidade de bits do codigo (0 = simbolo nao aparece).
 */
typedef struct {
    unsigned char bits[MAX_BYTES_CODIGO];
    int tamanho;
} Codigo;

/*
 * Dicionario completo.
 *   codigos[s]     : codigo do byte s
 *   frequencias[s] : frequencia do byte s (copiada das folhas da arvore)
 *   qtd_simbolos   : numero de simbolos distintos com codigo
 */
typedef struct {
    Codigo codigos[NUM_SIMBOLOS];
    unsigned long frequencias[NUM_SIMBOLOS];
    int qtd_simbolos;
} Dicionario;

/*
 * ETAPA 3: preenche 'dic' percorrendo a arvore 'arv'.
 * O Dicionario e fornecido por quem chama (variavel local, static ou
 * global), entao esta funcao NAO faz alocacao dinamica.
 * Retorna o numero de simbolos codificados.
 */
int dicionario_gerar(Dicionario *dic, const ArvoreHuffman *arv);

/* Quantidade de bits do codigo do simbolo s (0 se s nao aparece). */
int dicionario_tamanho(const Dicionario *dic, unsigned char s);

/* Valor (0 ou 1) do i-esimo bit do codigo do simbolo s. */
int dicionario_bit(const Dicionario *dic, unsigned char s, int i);

/* Total de bits do conteudo comprimido: soma de frequencia * tamanho
 * (sem contar o cabecalho). Obs.: no Windows unsigned long tem 32 bits,
 * o que cobre arquivos de ate ~500 MB. */
unsigned long dicionario_total_bits(const Dicionario *dic);

/* Media ponderada de bits por simbolo (sem compressao seria 8). */
double dicionario_media_bits(const Dicionario *dic);

/* Imprime a tabela Simbolo | Frequencia | Bits | Codigo. */
void dicionario_imprimir(const Dicionario *dic, FILE *saida);

#endif /* DICIONARIO_H */
