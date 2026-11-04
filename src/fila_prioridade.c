/*
 * ============================================================================
 *  fila_prioridade.c  -  Min-Heap (versao provisoria; a oficial e da Pessoa 1)
 * ============================================================================
 *  Heap binario guardado em vetor:
 *    pai(i) = (i-1)/2     filho_esq(i) = 2i+1     filho_dir(i) = 2i+2
 *  O menor elemento fica sempre na posicao 0.
 *  Insercao e remocao custam O(log n).
 * ============================================================================
 */
#include "../include/fila_prioridade.h"

/* Retorna 1 se 'a' deve sair da fila ANTES de 'b'.
 * O desempate compara os enderecos: todos os nos vem do mesmo vetor
 * (o "pool" da ArvoreHuffman), entao endereco menor = criado antes. */
static int tem_prioridade(const No *a, const No *b)
{
    if (a->frequencia != b->frequencia)
        return a->frequencia < b->frequencia;
    return a < b;
}

static void trocar(No **x, No **y)
{
    No *tmp = *x;
    *x = *y;
    *y = tmp;
}

void fila_iniciar(FilaPrioridade *f)
{
    f->tamanho = 0;
}

int fila_tamanho(const FilaPrioridade *f)
{
    return f->tamanho;
}

int fila_inserir(FilaPrioridade *f, No *no)
{
    int i;

    if (f->tamanho >= FILA_CAPACIDADE)
        return 0;

    /* Coloca no final e "sobe" enquanto tiver prioridade sobre o pai */
    i = f->tamanho++;
    f->itens[i] = no;
    while (i > 0 && tem_prioridade(f->itens[i], f->itens[(i - 1) / 2])) {
        trocar(&f->itens[i], &f->itens[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
    return 1;
}

No *fila_remover_min(FilaPrioridade *f)
{
    No *menor;
    int i, filho;

    if (f->tamanho == 0)
        return NULL;

    /* O menor esta na posicao 0; o ultimo elemento ocupa o lugar dele
     * e "desce" ate a posicao correta */
    menor = f->itens[0];
    f->itens[0] = f->itens[--f->tamanho];

    i = 0;
    for (;;) {
        filho = 2 * i + 1;                         /* filho da esquerda */
        if (filho >= f->tamanho)
            break;
        if (filho + 1 < f->tamanho &&
            tem_prioridade(f->itens[filho + 1], f->itens[filho]))
            filho++;                               /* o da direita e menor */
        if (!tem_prioridade(f->itens[filho], f->itens[i]))
            break;                                 /* ja esta no lugar */
        trocar(&f->itens[i], &f->itens[filho]);
        i = filho;
    }
    return menor;
}
