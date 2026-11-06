#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/compressao.h"     /* Etapa 4 */
#include "../include/descompressao.h"  /* Etapa 5 */

// ./testes/redundancia_alta.txt
// ./testes/redundancia_media.txt
// ./testes/redundancia_baixa.txt


void rotina_compressao() {
    char caminho_orig[256], caminho_dest[256];
    printf("\nCaminho do arquivo original: ");
    scanf("%255s", caminho_orig);
    printf("Nome do arquivo de saida: ");
    scanf("%255s", caminho_dest);

    // O metodo comprimir_arquivo já calcula e exibe as estatisticas
    if (comprimir_arquivo(caminho_orig, caminho_dest) == 0) {
        printf("\n=> Compressao realizada com sucesso!\n");
    } else {
        printf("\n=> Falha na compressao do arquivo.\n");
    }
}

void rotina_descompressao() {
    char caminho_orig[256], caminho_dest[256];
    printf("\nCaminho do arquivo comprimido: ");
    scanf("%255s", caminho_orig);
    printf("Nome do arquivo restaurado: ");
    scanf("%255s", caminho_dest);

    // O metodo descomprimir_arquivo já calcula e exibe o tempo
    if (descomprimir_arquivo(caminho_orig, caminho_dest) == 0) {
        printf("\n=> Descompressao realizada com sucesso!\n");
    } else {
        printf("\n=> Falha na descompressao do arquivo.\n");
    }
}

int main(int argc, char *argv[]) {

    if (argc == 4) {
        if (strcmp(argv[1], "c") == 0)
            return comprimir_arquivo(argv[2], argv[3]) == 0 ? 0 : 1;
        if (strcmp(argv[1], "d") == 0)
            return descomprimir_arquivo(argv[2], argv[3]) == 0 ? 0 : 1;
    }

    
    int opcao = 0;
    while (opcao != 3) {
        printf("\n==================================\n");
        printf("       Programa Compressão Zipz        \n");
        printf("==================================\n");
        printf("1) Comprimir\n");
        printf("2) Descomprimir\n");
        printf("3) Sair\n");
        printf("Escolha: ");
        
        if (scanf("%d", &opcao) != 1) {
            while(getchar() != '\n'); // Limpa buffer de caracteres
            continue;
        }

        if (opcao == 1) rotina_compressao();
        else if (opcao == 2) rotina_descompressao();
    }
    

    printf("Encerrando o programa...\n");
    return 0;
}