/*
 * ============================================================================
 *  fila_prioridade.h  -  Fila de Prioridade (Min-Heap) de ponteiros para No
 * ============================================================================
 *  ATENCAO: pela divisao do grupo, a Fila de Prioridade e da PESSOA 1.
 *  Esta versao foi feita para que a Etapa 2 (arvore.c) ja compile e possa
 *  ser testada. Se a Pessoa 1 tiver a dela, basta manter estes nomes de
 *  funcao ou ajustar as 4 chamadas feitas em src/arvore.c.
 *
 *  Criterio de prioridade (quem sai primeiro):
 *    1) menor frequencia;
 *    2) empate: o no que foi criado antes (posicao menor no vetor de nos
 *       da arvore). Assim a arvore gerada e sempre a mesma.
 * ============================================================================
 */
#ifndef FILA_PRIORIDADE_H
#define FILA_PRIORIDADE_H

#include "huffman.h"

/* Na construcao de Huffman nunca ha mais que 256 nos na fila ao mesmo tempo
 * (comeca com ate 256 folhas e o tamanho so diminui). Um vetor fixo basta:
 * a fila nao precisa de nenhuma alocacao dinamica. */
#define FILA_CAPACIDADE 256

typedef struct {
    No *itens[FILA_CAPACIDADE];
    int tamanho;
} FilaPrioridade;

void fila_iniciar(FilaPrioridade *f);
int  fila_inserir(FilaPrioridade *f, No *no);     /* 1 = ok, 0 = cheia */
No  *fila_remover_min(FilaPrioridade *f);         /* NULL se vazia     */
int  fila_tamanho(const FilaPrioridade *f);

#endif /* FILA_PRIORIDADE_H */
