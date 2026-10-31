#include "../include/huffman.h"
#include <stdlib.h>

// Funcao de Leitura do arquivo para contagem da ocorrência de cada símbolo
void calcular_frequencias(const char *caminho_arquivo, unsigned long *frequencias) {
    // Modo "rb" garante a leitura segura tanto de textos quanto de binários
    FILE *arquivo = fopen(caminho_arquivo, "rb");
    
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", caminho_arquivo);
        return;
    }

    int byte_lido;
    // O fgetc retorna um int para poder sinalizar o EOF (End Of File) de forma segura
    while ((byte_lido = fgetc(arquivo)) != EOF) {
        frequencias[byte_lido]++;
    }

    fclose(arquivo);
}