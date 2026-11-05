/*
 * ============================================================================
 *  teste_pessoa2.c  -  Demonstracao e verificacao das Etapas 2 e 3
 * ============================================================================
 *  Compilar (na pasta principal do projeto):
 *     gcc -Wall -Wextra -o teste_pessoa2 testes/teste_pessoa2.c
 *         src/huffman.c src/fila_prioridade.c src/arvore.c src/dicionario.c -lm
 *
 *  Usar:
 *     ./teste_pessoa2                 -> exemplo "ABRACADABRA"
 *     ./teste_pessoa2 <arquivo>       -> qualquer arquivo (texto ou binario)
 *
 *  O que o programa faz:
 *    1. conta as frequencias com calcular_frequencias()  (Etapa 1 - Pessoa 1)
 *    2. monta a Arvore de Huffman                         (Etapa 2 - Pessoa 2)
 *    3. gera o dicionario de codigos                      (Etapa 3 - Pessoa 2)
 *    4. VERIFICA sozinho se os codigos estao corretos:
 *         - todo simbolo que aparece tem codigo (e so eles);
 *         - nenhum codigo e prefixo de outro (propriedade de prefixo);
 *         - soma de Kraft: soma de 2^(-tamanho) == 1 (arvore sem desperdicio)
 *    5. mostra estatisticas: bits/simbolo, entropia, tamanho estimado.
 * ============================================================================
 */
#include <stdio.h>
#include <math.h>
#include "../include/huffman.h"
#include "../include/arvore.h"
#include "../include/dicionario.h"

/* Conta as frequencias de um texto fixo (usado no exemplo sem arquivo). */
static void contar_texto(const char *texto, unsigned long frequencias[])
{
    int i;
    for (i = 0; texto[i] != '\0'; i++)
        frequencias[(unsigned char) texto[i]]++;
}

/* O codigo do simbolo 'a' e o comeco (prefixo) do codigo de 'b'? */
static int eh_prefixo(const Dicionario *d, int a, int b)
{
    int i, ta = d->codigos[a].tamanho;

    if (ta > d->codigos[b].tamanho)
        return 0;
    for (i = 0; i < ta; i++)
        if (dicionario_bit(d, (unsigned char) a, i) !=
            dicionario_bit(d, (unsigned char) b, i))
            return 0;
    return 1;
}

/* Faz as tres verificacoes. Retorna 1 se tudo estiver certo. */
static int verificar(const Dicionario *d, const unsigned long frequencias[])
{
    int a, b;
    int ok_codigos = 1, ok_prefixo = 1, ok_kraft = 1;
    double kraft = 0.0;

    for (a = 0; a < NUM_SIMBOLOS; a++) {
        /* (1) quem aparece no arquivo tem codigo; quem nao aparece, nao */
        if ((frequencias[a] > 0) != (d->codigos[a].tamanho > 0))
            ok_codigos = 0;
        if (d->codigos[a].tamanho == 0)
            continue;

        kraft += pow(2.0, -(double) d->codigos[a].tamanho);

        /* (2) nenhum codigo pode ser o comeco de outro */
        for (b = 0; b < NUM_SIMBOLOS; b++)
            if (a != b && d->codigos[b].tamanho > 0 && eh_prefixo(d, a, b))
                ok_prefixo = 0;
    }

    /* (3) Kraft: arvore cheia => soma exatamente 1.
     *     Excecao: um unico simbolo (codigo "0", soma = 0.5). */
    if (d->qtd_simbolos > 1 && fabs(kraft - 1.0) > 1e-9)
        ok_kraft = 0;

    printf("  Todo simbolo presente tem codigo ...... %s\n", ok_codigos ? "OK" : "FALHOU");
    printf("  Nenhum codigo e prefixo de outro ...... %s\n", ok_prefixo ? "OK" : "FALHOU");
    printf("  Soma de Kraft = %.6f .............. %s\n", kraft, ok_kraft ? "OK" : "FALHOU");
    return ok_codigos && ok_prefixo && ok_kraft;
}

/* Entropia de Shannon: o minimo teorico de bits por simbolo. */
static double entropia(const unsigned long frequencias[], unsigned long total)
{
    double h = 0.0, p;
    int s;

    for (s = 0; s < NUM_SIMBOLOS; s++) {
        if (frequencias[s] == 0)
            continue;
        p = (double) frequencias[s] / (double) total;
        h -= p * (log(p) / log(2.0));
    }
    return h;
}

int main(int argc, char *argv[])
{
    unsigned long frequencias[NUM_SIMBOLOS] = {0};
    static Dicionario dic;        /* ~10 KB: static para nao pesar na pilha */
    ArvoreHuffman *arv;
    unsigned long total = 0, bits;
    int s, ok;

    /* ------------------------ Etapa 1 (Pessoa 1) ------------------------ */
    if (argc >= 2) {
        calcular_frequencias(argv[1], frequencias);
        printf("Arquivo: %s\n", argv[1]);
    } else {
        contar_texto("ABRACADABRA", frequencias);
        printf("Exemplo: \"ABRACADABRA\"\n");
    }
    for (s = 0; s < NUM_SIMBOLOS; s++)
        total += frequencias[s];
    printf("Total de simbolos (bytes): %lu\n\n", total);

    /* ------------------- Etapa 2: construcao da arvore ------------------ */
    arv = arvore_construir(frequencias);
    if (arv == NULL) {
        printf("Erro: memoria insuficiente\n");
        return 1;
    }
    printf("=== ETAPA 2: Arvore de Huffman ===\n");
    printf("Simbolos distintos: %d | Nos usados: %d | Altura: %d\n\n",
           arv->qtd_folhas, arv->qtd_nos, arvore_altura(arv));
    if (arv->qtd_folhas <= 16)            /* arvore grande polui a tela */
        arvore_imprimir(arv, stdout);
    else
        printf("(arvore grande demais para desenhar; veja a tabela)\n");

    /* ------------------- Etapa 3: geracao dos codigos ------------------- */
    dicionario_gerar(&dic, arv);
    printf("\n=== ETAPA 3: Dicionario de codigos ===\n");
    dicionario_imprimir(&dic, stdout);

    /* -------------------- Verificacao e estatisticas -------------------- */
    printf("\n=== Verificacao ===\n");
    ok = verificar(&dic, frequencias);

    bits = dicionario_total_bits(&dic);
    printf("\n=== Estatisticas ===\n");
    if (total > 0)
        printf("  Entropia (limite teorico) : %.4f bits/simbolo\n",
               entropia(frequencias, total));
    printf("  Huffman (media ponderada) : %.4f bits/simbolo\n",
           dicionario_media_bits(&dic));
    printf("  Sem compressao            : 8.0000 bits/simbolo\n");
    printf("  Dados comprimidos (sem cabecalho): %lu bits = %lu bytes\n",
           bits, (bits + 7) / 8);
    if (total > 0)
        printf("  Razao estimada (sem cabecalho)   : %.4f\n",
               (double) ((bits + 7) / 8) / (double) total);

    arvore_liberar(arv);                  /* 1 malloc -> 1 free */
    return ok ? 0 : 2;
}
