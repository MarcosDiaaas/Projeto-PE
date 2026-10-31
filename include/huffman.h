#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

// Estrutura dos nós da codificação (TAD)
typedef struct No {
    unsigned char simbolo;
    unsigned long frequencia;
    struct No *filhoesquerda;
    struct No *filhodireita;
} No;

// Função para calculo de frequencias (recebe ponteiro do arquivo e frequencia dos simbolos)
void calcular_frequencias(const char *caminho_arquivo, unsigned long *frequencias);

#endif