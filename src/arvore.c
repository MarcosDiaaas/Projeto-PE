/*
 * ============================================================================
 *  arvore.c  -  Implementacao do TAD Arvore de Codificacao (Huffman)
 * ============================================================================
 *  Responsavel: Pessoa 2 (Etapa 2 - Construcao do Modelo)
 *
 *  ALGORITMO DE HUFFMAN
 *  --------------------
 *   1. Para cada simbolo com frequencia > 0, cria uma FOLHA e coloca na
 *      fila de prioridade (min-heap ordenado pela frequencia).
 *   2. Enquanto houver mais de um no na fila:
 *        a) tira os DOIS nos de menor frequencia (x e y);
 *        b) cria um no interno z com frequencia(z) = freq(x) + freq(y),
 *           com x a esquerda (bit 0) e y a direita (bit 1);
 *        c) coloca z de volta na fila.
 *   3. O unico no que sobra na fila e a RAIZ da arvore.
 *
 *  COMPLEXIDADE
 *  ------------
 *   n = numero de simbolos distintos (n <= 256).
 *   - n-1 juncoes; cada uma faz 2 remocoes e 1 insercao na fila, cada
 *     operacao O(log n)  ->  tempo total O(n log n).
 *   - Memoria: O(n) nos, todos dentro do pool (1 unico malloc).
 *   Como n <= 256, esta etapa e praticamente instantanea; o custo real do
 *   programa esta em ler e gravar os arquivos (Etapas 1, 4 e 5).
 * ============================================================================
 */
#include <stdlib.h>
#include "../include/arvore.h"
#include "../include/fila_prioridade.h"

/* ========================================================================= */
/*                         Criacao / destruicao                              */
/* ========================================================================= */

ArvoreHuffman *arvore_criar_vazia(void)
{
    /* UNICA alocacao dinamica da arvore inteira (pool com 511 nos) */
    ArvoreHuffman *arv = (ArvoreHuffman *) malloc(sizeof(ArvoreHuffman));
    if (arv == NULL)
        return NULL;

    arv->qtd_nos = 0;
    arv->qtd_folhas = 0;
    arv->raiz = NULL;
    return arv;
}

void arvore_liberar(ArvoreHuffman *arv)
{
    /* Todos os nos estao dentro do pool: um free libera tudo */
    free(arv);
}

/* ========================================================================= */
/*                     Criacao de nos a partir do pool                       */
/* ========================================================================= */

/* Reserva o proximo no livre do pool. Como os nos sao usados em ordem,
 * a posicao no vetor tambem indica a ordem de criacao (usada no desempate
 * da fila de prioridade). */
static No *reservar_no(ArvoreHuffman *arv)
{
    if (arv->qtd_nos >= MAX_NOS)
        return NULL;                        /* nunca deveria ocorrer */
    return &arv->nos[arv->qtd_nos++];
}

No *arvore_novo_folha(ArvoreHuffman *arv, unsigned char simbolo,
                      unsigned long frequencia)
{
    No *no = reservar_no(arv);
    if (no == NULL)
        return NULL;

    no->simbolo = simbolo;
    no->frequencia = frequencia;
    no->filhoesquerda = NULL;               /* folha: sem filhos */
    no->filhodireita = NULL;
    arv->qtd_folhas++;
    return no;
}

No *arvore_novo_interno(ArvoreHuffman *arv, No *esq, No *dir)
{
    No *no = reservar_no(arv);
    if (no == NULL)
        return NULL;

    no->simbolo = 0;                        /* sem significado em no interno */
    no->filhoesquerda = esq;
    no->filhodireita = dir;
    /* frequencia do pai = soma das frequencias dos filhos */
    no->frequencia = (esq ? esq->frequencia : 0) + (dir ? dir->frequencia : 0);
    return no;
}

/* ========================================================================= */
/*                    ETAPA 2 - Construcao da arvore                         */
/* ========================================================================= */

ArvoreHuffman *arvore_construir(const unsigned long frequencias[NUM_SIMBOLOS])
{
    ArvoreHuffman *arv;
    FilaPrioridade fila;          /* fica na pilha: nenhum malloc */
    No *x, *y, *z;
    int s;

    arv = arvore_criar_vazia();
    if (arv == NULL)
        return NULL;

    fila_iniciar(&fila);

    /* --- Passo 1: uma folha para cada simbolo que aparece no arquivo --- */
    for (s = 0; s < NUM_SIMBOLOS; s++) {
        if (frequencias[s] > 0) {
            x = arvore_novo_folha(arv, (unsigned char) s, frequencias[s]);
            fila_inserir(&fila, x);
        }
    }

    /* --- Caso especial A: arquivo vazio -> arvore sem raiz --- */
    if (fila_tamanho(&fila) == 0)
        return arv;

    /* --- Caso especial B: um unico simbolo distinto (ex.: "aaaaaa") ---
     * Se a folha fosse a propria raiz, o codigo do simbolo teria 0 bits e
     * nada seria gravado. Por isso criamos uma raiz artificial com a folha
     * a ESQUERDA -> o simbolo recebe o codigo "0" (1 bit). */
    if (fila_tamanho(&fila) == 1) {
        x = fila_remover_min(&fila);
        arv->raiz = arvore_novo_interno(arv, x, NULL);
        return arv;
    }

    /* --- Passo 2: junta sempre os dois de menor frequencia --- */
    while (fila_tamanho(&fila) > 1) {
        x = fila_remover_min(&fila);        /* menor frequencia       */
        y = fila_remover_min(&fila);        /* segunda menor          */
        z = arvore_novo_interno(arv, x, y); /* x -> bit 0, y -> bit 1 */
        fila_inserir(&fila, z);
    }

    /* --- Passo 3: o que sobrou e a raiz --- */
    arv->raiz = fila_remover_min(&fila);
    return arv;
}

/* ========================================================================= */
/*                               Consultas                                   */
/* ========================================================================= */

int arvore_eh_folha(const No *no)
{
    return no != NULL && no->filhoesquerda == NULL && no->filhodireita == NULL;
}

/* Altura medida em arestas (folha = 0). */
static int altura_rec(const No *no)
{
    int he, hd;

    if (no == NULL || arvore_eh_folha(no))
        return 0;

    he = no->filhoesquerda ? altura_rec(no->filhoesquerda) : 0;
    hd = no->filhodireita ? altura_rec(no->filhodireita) : 0;
    return 1 + (he > hd ? he : hd);
}

int arvore_altura(const ArvoreHuffman *arv)
{
    if (arv == NULL || arv->raiz == NULL)
        return 0;
    return altura_rec(arv->raiz);
}

/* ========================================================================= */
/*                               Exibicao                                    */
/* ========================================================================= */

void arvore_imprimir_simbolo(FILE *saida, unsigned char simbolo)
{
    /* Caracteres de controle e bytes fora do ASCII (acentos em UTF-8, por
     * exemplo) sao mostrados em hexadecimal para nao baguncar o terminal */
    switch (simbolo) {
    case '\n': fprintf(saida, "'\\n'"); break;
    case '\r': fprintf(saida, "'\\r'"); break;
    case '\t': fprintf(saida, "'\\t'"); break;
    case ' ':  fprintf(saida, "' '");   break;
    default:
        if (simbolo >= 33 && simbolo <= 126)
            fprintf(saida, "'%c'", simbolo);
        else
            fprintf(saida, "0x%02X", simbolo);
    }
}

/* Percorre direita -> no -> esquerda, recuando conforme a profundidade,
 * para a arvore aparecer "deitada": raiz a esquerda, ramo 1 em cima e
 * ramo 0 embaixo. O rotulo [0]/[1] e o bit da aresta que leva ao no. */
static void imprimir_rec(const No *no, int prof, int bit, FILE *saida)
{
    int i;

    if (no == NULL)
        return;

    imprimir_rec(no->filhodireita, prof + 1, 1, saida);

    for (i = 0; i < prof; i++)
        fprintf(saida, "        ");
    if (prof > 0)
        fprintf(saida, "[%d]-", bit);

    if (arvore_eh_folha(no)) {
        arvore_imprimir_simbolo(saida, no->simbolo);
        fprintf(saida, ":%lu\n", no->frequencia);
    } else {
        fprintf(saida, "(%lu)\n", no->frequencia);
    }

    imprimir_rec(no->filhoesquerda, prof + 1, 0, saida);
}

void arvore_imprimir(const ArvoreHuffman *arv, FILE *saida)
{
    if (arv == NULL || arv->raiz == NULL) {
        fprintf(saida, "(arvore vazia)\n");
        return;
    }
    imprimir_rec(arv->raiz, 0, 0, saida);
}
