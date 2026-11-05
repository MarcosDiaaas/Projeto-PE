#include <stdio.h>
#include <time.h>
#include "../include/descompressao.h"
#include "../include/arvore.h"       /* arvore_construir() - mesma árvore da Etapa 2 */

#define TAM_BLOCO 65536

/* ---------------------------------------------------------------------
 * Leitor de bits
 * Lê um byte do arquivo e entrega seus bits do mais significativo para
 * o menos significativo (mesma ordem em que a compressão gravou).
 * --------------------------------------------------------------------- */
void leitor_inicia(LeitorBits *L, FILE *arq) {
    L->arq = arq;
    L->buffer = 0;
    L->nBits = 0;
}

int leitor_bit(LeitorBits *L) {
    if (L->nBits == 0) {
        int c = fgetc(L->arq);
        if (c == EOF) return -1;
        L->buffer = (unsigned char)c;
        L->nBits = 8;
    }
    L->nBits--;
    return (L->buffer >> L->nBits) & 1;
}

/* ---------------------------------------------------------------------
 * Cabeçalho (formato descrito em compressao.h)
 * --------------------------------------------------------------------- */
static int ler_uint32(FILE *f, unsigned long *v) {
    int i, c;
    *v = 0;
    for (i = 0; i < 4; i++) {
        if ((c = fgetc(f)) == EOF) return -1;
        *v = (*v << 8) | (unsigned long)c;
    }
    return 0;
}

int ler_cabecalho(FILE *entrada, unsigned long freq[NUM_SIMBOLOS],
                  unsigned long *total_bytes) {
    int i, n, c1, c2, sim;
    unsigned long f, soma = 0;

    /* assinatura */
    if (fgetc(entrada) != 'Z' || fgetc(entrada) != 'I' ||
        fgetc(entrada) != 'P' || fgetc(entrada) != 'Z')
        return -1;

    /* tamanho original */
    if (ler_uint32(entrada, total_bytes) != 0) return -1;

    /* número de símbolos distintos */
    if ((c1 = fgetc(entrada)) == EOF || (c2 = fgetc(entrada)) == EOF) return -1;
    n = (c1 << 8) | c2;
    if (n > NUM_SIMBOLOS) return -1;

    /* tabela símbolo -> frequência */
    for (i = 0; i < NUM_SIMBOLOS; i++) freq[i] = 0;
    for (i = 0; i < n; i++) {
        if ((sim = fgetc(entrada)) == EOF) return -1;
        if (ler_uint32(entrada, &f) != 0) return -1;
        if (f == 0 || freq[sim] != 0) return -1;   /* frequência nula ou símbolo repetido */
        freq[sim] = f;
        soma += f;
        if (soma < f) return -1;                   /* estourou: cabeçalho inválido */
    }

    /* cada byte do original foi contado exatamente uma vez */
    if (soma != *total_bytes) return -1;
    return 0;
}

/* ---------------------------------------------------------------------
 * Decodificação: anda na árvore bit a bit.
 * Os bytes decodificados vão para um bloco na memória e são gravados de
 * TAM_BLOCO em TAM_BLOCO (bem mais rápido que um fputc por byte).
 * --------------------------------------------------------------------- */
int decodificar_arquivo(FILE *entrada, FILE *saida, const ArvoreHuffman *arv,
                        unsigned long total_bytes) {
    static unsigned char bloco[TAM_BLOCO];
    size_t usados = 0;
    unsigned long decodificados = 0;
    const No *atual;
    LeitorBits L;
    int bit;

    if (total_bytes == 0) return 0;               /* arquivo vazio: nada a fazer */
    if (arv == NULL || arv->raiz == NULL) return -1;

    leitor_inicia(&L, entrada);
    atual = arv->raiz;

    while (decodificados < total_bytes) {
        bit = leitor_bit(&L);
        if (bit < 0) return -1;                   /* acabaram os bits antes da hora */

        atual = bit ? atual->filhodireita : atual->filhoesquerda;
        if (atual == NULL) return -1;             /* caminho que não existe na árvore */

        if (arvore_eh_folha(atual)) {
            bloco[usados++] = atual->simbolo;
            decodificados++;
            atual = arv->raiz;
            if (usados == TAM_BLOCO) {
                if (fwrite(bloco, 1, usados, saida) != usados) return -1;
                usados = 0;
            }
        }
    }
    if (usados > 0 && fwrite(bloco, 1, usados, saida) != usados) return -1;

    /* depois do último byte de dados não pode sobrar nada no arquivo */
    if (fgetc(entrada) != EOF) return -1;

    return ferror(saida) ? -1 : 0;
}

/* ---------------------------------------------------------------------
 * Utilitários
 * --------------------------------------------------------------------- */
static long tamanho_de(FILE *f) {
    long tam;
    fflush(f);
    if (fseek(f, 0, SEEK_END) != 0) return -1;
    tam = ftell(f);
    rewind(f);
    return tam;
}

static int caminhos_iguais(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) { a++; b++; }
    return *a == *b;
}

/* ---------------------------------------------------------------------
 * Descompressão completa
 * --------------------------------------------------------------------- */
int descomprimir_arquivo(const char *caminho_entrada, const char *caminho_saida) {
    unsigned long freq[NUM_SIMBOLOS];
    unsigned long total;
    ArvoreHuffman *arv;
    FILE *entrada, *saida;
    long tam_comprimido, tam_descomprimido;
    clock_t inicio = clock();

    if (caminhos_iguais(caminho_entrada, caminho_saida)) {
        printf("Erro: o arquivo de saida deve ser diferente do comprimido.\n");
        return -1;
    }

    entrada = fopen(caminho_entrada, "rb");
    if (entrada == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", caminho_entrada);
        return -1;
    }
    tam_comprimido = tamanho_de(entrada);

    /* 1. cabeçalho */
    if (ler_cabecalho(entrada, freq, &total) != 0) {
        printf("Erro: %s nao e um arquivo ZIPZ valido (ou esta corrompido).\n",
               caminho_entrada);
        fclose(entrada);
        return -1;
    }

    /* 2. mesma árvore da compressão */
    arv = arvore_construir(freq);
    if (arv == NULL) {
        printf("Erro: memoria insuficiente para a arvore.\n");
        fclose(entrada);
        return -1;
    }

    /* 3 e 4. bits -> bytes */
    saida = fopen(caminho_saida, "wb");
    if (saida == NULL) {
        printf("Erro ao criar o arquivo: %s\n", caminho_saida);
        arvore_liberar(arv);
        fclose(entrada);
        return -1;
    }

    if (decodificar_arquivo(entrada, saida, arv, total) != 0) {
        printf("Erro durante a decodificacao: arquivo comprimido corrompido.\n");
        arvore_liberar(arv);
        fclose(entrada); fclose(saida); remove(caminho_saida);
        return -1;
    }
    arvore_liberar(arv);

    tam_descomprimido = tamanho_de(saida);
    fclose(entrada);
    if (fclose(saida) != 0) {
        printf("Erro ao finalizar o arquivo de saida.\n");
        remove(caminho_saida);
        return -1;
    }

    printf("Arquivo comprimido: %s    Tamanho: %ld Bytes\n", caminho_entrada, tam_comprimido);
    printf("Arquivo descomprimido: %s    Tamanho: %ld Bytes\n", caminho_saida, tam_descomprimido);
    printf("Tempo de descompressao: %.3f s\n",
           (double)(clock() - inicio) / CLOCKS_PER_SEC);
    return 0;
}
