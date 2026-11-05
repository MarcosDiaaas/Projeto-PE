#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/compressao.h"
#include "../include/huffman.h"      /* calcular_frequencias()  - Etapa 1 */
#include "../include/arvore.h"       /* arvore_construir()      - Etapa 2 */
#include "../include/dicionario.h"   /* dicionario_gerar()      - Etapa 3 */

#define TAM_BLOCO 65536

/* ---------------------------------------------------------------------
 * Escritor de bits
 * --------------------------------------------------------------------- */
void escritor_inicia(EscritorBits *E, FILE *arq) {
    E->arq = arq;
    E->buffer = 0;
    E->nBits = 0;
    E->bytes_gravados = 0;
}

void escritor_bit(EscritorBits *E, int bit) {
    E->buffer = (unsigned char)((E->buffer << 1) | (bit & 1));
    E->nBits++;
    if (E->nBits == 8) {
        fputc(E->buffer, E->arq);
        E->bytes_gravados++;
        E->buffer = 0;
        E->nBits = 0;
    }
}

/* Escreve o código de um símbolo. O dicionário guarda os bits já
 * compactados (bit i no byte i/8, posição 7 - i%8), do mesmo jeito que o
 * arquivo precisa deles. Se o buffer está vazio, os bytes completos do
 * código são gravados direto; os bits que sobram vão um a um. */
void escritor_codigo(EscritorBits *E, const Codigo *c) {
    int i = 0, tam = c->tamanho;

    if (E->nBits == 0) {
        for (; i + 8 <= tam; i += 8) {
            fputc(c->bits[i / 8], E->arq);
            E->bytes_gravados++;
        }
    }
    for (; i < tam; i++)
        escritor_bit(E, (c->bits[i / 8] >> (7 - (i % 8))) & 1);
}

/* Completa o último byte com zeros e grava. */
void escritor_finaliza(EscritorBits *E) {
    if (E->nBits > 0) {
        E->buffer = (unsigned char)(E->buffer << (8 - E->nBits));
        fputc(E->buffer, E->arq);
        E->bytes_gravados++;
        E->buffer = 0;
        E->nBits = 0;
    }
}

/* ---------------------------------------------------------------------
 * Cabeçalho (formato descrito em compressao.h)
 * --------------------------------------------------------------------- */
static void gravar_uint32(FILE *f, unsigned long v) {
    fputc((int)((v >> 24) & 0xFFUL), f);
    fputc((int)((v >> 16) & 0xFFUL), f);
    fputc((int)((v >> 8) & 0xFFUL), f);
    fputc((int)(v & 0xFFUL), f);
}

int gravar_cabecalho(FILE *saida, const unsigned long freq[NUM_SIMBOLOS],
                     unsigned long total_bytes) {
    int i, n = 0;

    if (total_bytes > 0xFFFFFFFFUL) return -1;       /* não cabe em 4 bytes */
    for (i = 0; i < NUM_SIMBOLOS; i++)
        if (freq[i] > 0) {
            if (freq[i] > 0xFFFFFFFFUL) return -1;
            n++;
        }

    fputc('Z', saida); fputc('I', saida); fputc('P', saida); fputc('Z', saida);
    gravar_uint32(saida, total_bytes);
    fputc((n >> 8) & 0xFF, saida);
    fputc(n & 0xFF, saida);
    for (i = 0; i < NUM_SIMBOLOS; i++) {
        if (freq[i] > 0) {
            fputc(i, saida);
            gravar_uint32(saida, freq[i]);
        }
    }
    return ferror(saida) ? -1 : 0;
}

/* ---------------------------------------------------------------------
 * Segunda leitura do arquivo: troca cada byte pelo seu código.
 * 'entrada' já deve estar no início (rewind).
 * --------------------------------------------------------------------- */
int codificar_arquivo(FILE *entrada, FILE *saida, const Dicionario *dic,
                      unsigned long total_esperado) {
    unsigned char bloco[TAM_BLOCO];
    size_t lidos, i;
    unsigned long total = 0;
    EscritorBits E;

    escritor_inicia(&E, saida);
    while ((lidos = fread(bloco, 1, TAM_BLOCO, entrada)) > 0) {
        for (i = 0; i < lidos; i++) {
            const Codigo *c = &dic->codigos[bloco[i]];
            if (c->tamanho == 0) return -1;   /* byte sem código: arquivo mudou */
            escritor_codigo(&E, c);
        }
        total += (unsigned long)lidos;
    }
    escritor_finaliza(&E);
    if (total != total_esperado) return -1;
    return ferror(saida) ? -1 : 0;
}

/* ---------------------------------------------------------------------
 * Utilitários
 * --------------------------------------------------------------------- */
long tamanho_arquivo(FILE *f) {
    long tam;
    fflush(f);
    if (fseek(f, 0, SEEK_END) != 0) return -1;
    tam = ftell(f);
    rewind(f);
    return tam;
}

void exibir_estatisticas(const char *nome_original, const char *nome_comprimido,
                         long tam_original, long tam_comprimido, double segundos) {
    double razao = (tam_original > 0) ? (double)tam_comprimido / (double)tam_original : 0.0;
    printf("Arquivo original: %s    Tamanho: %ld Bytes\n", nome_original, tam_original);
    printf("Arquivo comprimido: %s    Tamanho: %ld Bytes\n", nome_comprimido, tam_comprimido);
    printf("Razao de compressao: %.4f\n", razao);
    printf("Tempo de compressao: %.3f s\n", segundos);
}

static int caminhos_iguais(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) { a++; b++; }
    return *a == *b;
}

/* ---------------------------------------------------------------------
 * Compressão completa
 * --------------------------------------------------------------------- */
int comprimir_arquivo(const char *caminho_entrada, const char *caminho_saida) {
    unsigned long freq[NUM_SIMBOLOS];
    Dicionario dic;
    ArvoreHuffman *arv;
    FILE *entrada, *saida;
    long tam_original, tam_comprimido;
    unsigned long total = 0;
    int i, n_simbolos = 0;
    clock_t inicio = clock();

    if (caminhos_iguais(caminho_entrada, caminho_saida)) {
        printf("Erro: o arquivo de saida deve ser diferente do original.\n");
        return -1;
    }

    entrada = fopen(caminho_entrada, "rb");
    if (entrada == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", caminho_entrada);
        return -1;
    }
    tam_original = tamanho_arquivo(entrada);

    /* ===== ETAPA 1 - frequências (calcular_frequencias) ===== */
    for (i = 0; i < NUM_SIMBOLOS; i++) freq[i] = 0;
    calcular_frequencias(caminho_entrada, freq);
    for (i = 0; i < NUM_SIMBOLOS; i++) {
        total += freq[i];
        if (freq[i] > 0) n_simbolos++;
    }
    /* calcular_frequencias não avisa quando falha: conferimos a soma */
    if (tam_original < 0 || total != (unsigned long)tam_original) {
        printf("Erro: contagem de frequencias inconsistente.\n");
        fclose(entrada);
        return -1;
    }

    /* ===== ETAPA 2 - árvore de Huffman (arvore_construir) ===== */
    arv = arvore_construir(freq);
    if (arv == NULL) {
        printf("Erro: memoria insuficiente para a arvore.\n");
        fclose(entrada);
        return -1;
    }

    /* ===== ETAPA 3 - códigos (dicionario_gerar) ===== */
    if (dicionario_gerar(&dic, arv) != n_simbolos) {
        printf("Erro ao gerar o dicionario de codigos.\n");
        arvore_liberar(arv);
        fclose(entrada);
        return -1;
    }
    arvore_liberar(arv);       /* o dicionário já tem tudo que a codificação usa */

    /* ===== ETAPA 4 - CODIFICAÇÃO ===== */
    saida = fopen(caminho_saida, "wb");
    if (saida == NULL) {
        printf("Erro ao criar o arquivo: %s\n", caminho_saida);
        fclose(entrada);
        return -1;
    }

    if (gravar_cabecalho(saida, freq, total) != 0) {
        printf("Erro ao gravar o cabecalho.\n");
        fclose(entrada); fclose(saida); remove(caminho_saida);
        return -1;
    }
    rewind(entrada);
    if (codificar_arquivo(entrada, saida, &dic, total) != 0) {
        printf("Erro durante a codificacao.\n");
        fclose(entrada); fclose(saida); remove(caminho_saida);
        return -1;
    }

    tam_comprimido = tamanho_arquivo(saida);
    fclose(entrada);
    if (fclose(saida) != 0) {
        printf("Erro ao finalizar o arquivo de saida.\n");
        remove(caminho_saida);
        return -1;
    }

    exibir_estatisticas(caminho_entrada, caminho_saida, tam_original, tam_comprimido,
                        (double)(clock() - inicio) / CLOCKS_PER_SEC);
    return 0;
}
