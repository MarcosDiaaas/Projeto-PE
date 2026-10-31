#include <stdio.h>
#include "../include/huffman.h"

int main() {

    /* TESTE DE FREQUENCIA

    // Cada posição do vetor representa um digito ASCII, assim ele recebe só a frequencia de cada simbolo.
    unsigned long frequencias[256] = {0};
    
    const char *arquivo_teste = "testes/redundancia_baixa.txt";
    
    printf("Analisando frequencias do arquivo: %s\n", arquivo_teste);
    calcular_frequencias(arquivo_teste, frequencias);
    
    // Laço pra printar a frequencia de cada caractere
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            // Imprime o código ASCII e o caractere visual correspondente
            printf("ASCII %3d (%c): %lu vezes\n", i, (char)i, frequencias[i]);
        }
    }

    */
   
    return 0;
}