/*
 * ============================================================================
 *  arvore.h  -  TAD Arvore de Codificacao (Arvore de Huffman)
 * ============================================================================
 *  Responsavel: Pessoa 2 (Etapa 2 - Construcao do Modelo)
 *
 *  O QUE ESTE MODULO FAZ
 *  ---------------------
 *  Recebe o vetor de frequencias da Etapa 1 (calcular_frequencias, em
 *  huffman.c) e monta a Arvore de Huffman, uma arvore binaria em que:
 *    - cada FOLHA guarda um simbolo (um byte, 0..255) e sua frequencia;
 *    - cada NO INTERNO guarda a soma das frequencias dos dois filhos.
 *  Simbolos frequentes ficam perto da raiz (codigo curto) e simbolos raros
 *  ficam longe (codigo longo).
 *
 *  Os nos sao do tipo 'No', definido em huffman.h:
 *    simbolo, frequencia, filhoesquerda, filhodireita.
 *  Convencao: ir para filhoesquerda = bit 0, ir para filhodireita = bit 1.
 *
 *  DECISAO DE PROJETO: "pool" de nos (uma unica alocacao)
 *  ------------------------------------------------------
 *  Com 256 simbolos possiveis, a arvore tem NO MAXIMO 2*256 - 1 = 511 nos
 *  (256 folhas + 255 internos). Em vez de um malloc por no (ate 511
 *  mallocs e 511 frees), todos os nos ficam num vetor dentro da struct:
 *    -> 1 malloc para criar a arvore e 1 free para destruir;
 *    -> nao existe no "esquecido" que vire memory leak;
 *    -> nos contiguos na memoria (melhor uso de cache).
 *
 *  O mesmo TAD serve para a DESCOMPRESSAO (Pessoa 4):
 *    - se o cabecalho guardar as frequencias, basta chamar
 *      arvore_construir() de novo: sai exatamente a mesma arvore;
 *    - se o cabecalho guardar a arvore, ela pode ser remontada com
 *      arvore_criar_vazia() + arvore_novo_folha() + arvore_novo_interno().
 * ============================================================================
 */
#ifndef ARVORE_H
#define ARVORE_H

#include <stdio.h>
#include "huffman.h"

/* Trabalhamos em nivel de BYTE: serve para texto e para arquivo binario */
#define NUM_SIMBOLOS 256

/* Maximo de nos de uma arvore binaria cheia com 256 folhas */
#define MAX_NOS (2 * NUM_SIMBOLOS - 1)

/*
 * A arvore inteira.
 *   nos        : "pool" com espaco para todos os nos possiveis
 *   qtd_nos    : quantos nos do pool ja foram usados
 *   qtd_folhas : quantos simbolos distintos existem no arquivo
 *   raiz       : ponteiro para a raiz (NULL se o arquivo estiver vazio)
 */
typedef struct {
    No nos[MAX_NOS];
    int qtd_nos;
    int qtd_folhas;
    No *raiz;
} ArvoreHuffman;

/* ---------------------------- Criacao / destruicao ----------------------- */

/* Aloca uma arvore vazia (sem nos). Retorna NULL se faltar memoria. */
ArvoreHuffman *arvore_criar_vazia(void);

/*
 * ETAPA 2: monta a Arvore de Huffman a partir das frequencias.
 *   frequencias[s] = quantas vezes o byte s aparece no arquivo.
 * Retorna a arvore (liberar depois com arvore_liberar) ou NULL se faltar
 * memoria. Se todas as frequencias forem zero (arquivo vazio), a arvore
 * e criada com raiz == NULL.
 */
ArvoreHuffman *arvore_construir(const unsigned long frequencias[NUM_SIMBOLOS]);

/* Libera toda a memoria da arvore (um unico free, gracas ao pool). */
void arvore_liberar(ArvoreHuffman *arv);

/* ------------------------- Criacao de nos (pool) ------------------------- */

/* Pega um no livre do pool e o inicializa como folha. NULL se pool cheio. */
No *arvore_novo_folha(ArvoreHuffman *arv, unsigned char simbolo,
                      unsigned long frequencia);

/* Pega um no livre do pool e o inicializa como no interno (pai de esq/dir).
 * dir pode ser NULL (caso especial de arquivo com um unico simbolo). */
No *arvore_novo_interno(ArvoreHuffman *arv, No *esq, No *dir);

/* ------------------------------- Consultas ------------------------------- */

/* 1 se o no e folha (nao tem filhos), 0 caso contrario. */
int arvore_eh_folha(const No *no);

/* Altura da arvore em arestas = tamanho do MAIOR codigo gerado. */
int arvore_altura(const ArvoreHuffman *arv);

/* ------------------------------- Exibicao -------------------------------- */

/* Escreve um simbolo de forma legivel: 'a', '\n', ' ', 0xC3 ... */
void arvore_imprimir_simbolo(FILE *saida, unsigned char simbolo);

/* Desenha a arvore "deitada" (raiz a esquerda), util na apresentacao. */
void arvore_imprimir(const ArvoreHuffman *arv, FILE *saida);

#endif /* ARVORE_H */
