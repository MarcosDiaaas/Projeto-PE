/* ============================================================================
 *  verificar_compressao.c - confere se um arquivo comprimido VOLTA ao original
 *
 *  É só um conferidor de teste, não o descompressor do programa. Ele lê o
 *  .zipz, reconstrói a árvore (arvore_construir) e compara, byte a byte, o
 *  que sai com o arquivo original.
 *
 *  Compilar (na pasta do projeto, uma linha só):
 *    gcc -Wall -Wextra -std=c99 -Iinclude testes/verificar_compressao.c
 *        src/arvore.c src/fila_prioridade.c -o verificar
 *  Usar:
 *    verificar original.txt original.txt.zipz
 * ============================================================================ */
#include <stdio.h>
#include "../include/arvore.h"

static unsigned long ler_uint32(FILE *f) {
    unsigned long v = 0;
    int i;
    for (i = 0; i < 4; i++) v = (v << 8) | (unsigned long)(fgetc(f) & 0xFF);
    return v;
}

int main(int argc, char *argv[]) {
    FILE *orig, *comp;
    unsigned long freq[NUM_SIMBOLOS], total, decodificados = 0;
    int i, n, c, b, sim, esperado;
    ArvoreHuffman *arv;
    const No *atual;

    if (argc != 3) {
        printf("Uso: %s <original> <comprimido>\n", argv[0]);
        return 2;
    }
    orig = fopen(argv[1], "rb");
    comp = fopen(argv[2], "rb");
    if (orig == NULL || comp == NULL) {
        printf("FALHOU: nao consegui abrir os arquivos.\n");
        return 2;
    }

    if (fgetc(comp) != 'Z' || fgetc(comp) != 'I' || fgetc(comp) != 'P' || fgetc(comp) != 'Z') {
        printf("FALHOU: assinatura ZIPZ invalida.\n");
        return 1;
    }
    total = ler_uint32(comp);
    n = (fgetc(comp) << 8);
    n |= fgetc(comp) & 0xFF;
    for (i = 0; i < NUM_SIMBOLOS; i++) freq[i] = 0;
    for (i = 0; i < n; i++) {
        sim = fgetc(comp);
        freq[sim & 0xFF] = ler_uint32(comp);
    }

    arv = arvore_construir(freq);
    if (arv == NULL) { printf("FALHOU: sem memoria.\n"); return 2; }
    atual = arv->raiz;

    while (decodificados < total && (c = fgetc(comp)) != EOF) {
        for (b = 7; b >= 0 && decodificados < total; b--) {
            atual = ((c >> b) & 1) ? atual->filhodireita : atual->filhoesquerda;
            if (atual == NULL) { printf("FALHOU: caminho invalido na arvore.\n"); return 1; }
            if (arvore_eh_folha(atual)) {
                esperado = fgetc(orig);
                if (esperado != (int)atual->simbolo) {
                    printf("FALHOU: diferenca no byte %lu.\n", decodificados);
                    return 1;
                }
                decodificados++;
                atual = arv->raiz;
            }
        }
    }
    arvore_liberar(arv);

    if (decodificados != total || fgetc(orig) != EOF) {
        printf("FALHOU: tamanhos diferentes (%lu de %lu bytes).\n", decodificados, total);
        return 1;
    }
    printf("OK: o arquivo comprimido volta identico ao original (%lu bytes).\n", total);
    fclose(orig);
    fclose(comp);
    return 0;
}
