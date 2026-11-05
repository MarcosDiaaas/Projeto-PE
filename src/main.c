#include <stdio.h>
#include <string.h>
#include "../include/compressao.h"     /* Etapa 4 */
#include "../include/descompressao.h"  /* Etapa 5 */

/*
 * Uso:
 *   main c <arquivo_original>   <arquivo_comprimido>    -> comprime
 *   main d <arquivo_comprimido> <arquivo_descomprimido> -> descomprime
 */
static void mostrar_uso(const char *programa) {
    printf("Uso:\n");
    printf("  %s c <original> <comprimido>      (comprimir)\n", programa);
    printf("  %s d <comprimido> <descomprimido> (descomprimir)\n", programa);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        mostrar_uso(argv[0]);
        return 2;
    }

    if (strcmp(argv[1], "c") == 0)
        return comprimir_arquivo(argv[2], argv[3]) == 0 ? 0 : 1;

    if (strcmp(argv[1], "d") == 0)
        return descomprimir_arquivo(argv[2], argv[3]) == 0 ? 0 : 1;

    mostrar_uso(argv[0]);
    return 2;
}
