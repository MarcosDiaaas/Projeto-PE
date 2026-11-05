/*
 * ============================================================================
 *  dicionario.c  -  Implementacao do TAD Dicionario (Etapa 3)
 * ============================================================================
 *  Responsavel: Pessoa 2
 *
 *  COMO OS CODIGOS SAO GERADOS
 *  ---------------------------
 *  Busca em profundidade (DFS) recursiva, a partir da raiz:
 *    - o caminho atual fica no vetor 'caminho' (um bit por posicao);
 *    - ao descer para a ESQUERDA escrevemos 0 na posicao 'prof';
 *    - ao descer para a DIREITA  escrevemos 1 na posicao 'prof';
 *    - ao chegar numa FOLHA, os 'prof' primeiros bits do caminho sao o
 *      codigo daquele simbolo -> copiamos (compactando) para o dicionario.
 *  O mesmo vetor 'caminho' e reaproveitado em toda a recursao: quando uma
 *  chamada volta, a proxima escrita simplesmente sobrescreve a posicao.
 *
 *  COMPLEXIDADE
 *  ------------
 *   - Cada no da arvore e visitado uma unica vez: O(numero de nos) = O(n).
 *   - Copiar o codigo na folha custa O(tamanho do codigo).
 *   - Profundidade maxima da recursao = altura da arvore (<= 255).
 *   - Memoria extra: so o vetor 'caminho' (256 bytes). Nenhum malloc.
 * ============================================================================
 */
#include "../include/dicionario.h"

/* ========================================================================= */
/*                          Funcoes auxiliares                               */
/* ========================================================================= */

/* Zera o dicionario inteiro (faz o papel do memset, sem usar string.h). */
static void dicionario_zerar(Dicionario *dic)
{
    int s, b;

    for (s = 0; s < NUM_SIMBOLOS; s++) {
        dic->codigos[s].tamanho = 0;
        dic->frequencias[s] = 0;
        for (b = 0; b < MAX_BYTES_CODIGO; b++)
            dic->codigos[s].bits[b] = 0;
    }
    dic->qtd_simbolos = 0;
}

/* Copia o caminho (1 bit por posicao) para o Codigo (8 bits por byte).
 * Para o bit i:
 *    byte    = i / 8            (em qual byte ele fica)
 *    mascara = 0x80 >> (i % 8)  (qual bit do byte; 0x80 = 1000 0000)
 *    bits[byte] |= mascara      (liga aquele bit)                         */
static void gravar_codigo(Codigo *cod, const unsigned char caminho[], int tam)
{
    int i;

    cod->tamanho = tam;
    for (i = 0; i < tam; i++) {
        if (caminho[i])
            cod->bits[i / 8] |= (unsigned char) (0x80 >> (i % 8));
    }
}

/* DFS recursiva: 'prof' = profundidade atual = quantos bits ja temos. */
static void percorrer(const No *no, unsigned char caminho[], int prof,
                      Dicionario *dic)
{
    if (no == NULL)
        return;

    /* Chegou numa folha: o caminho ate aqui e o codigo do simbolo */
    if (arvore_eh_folha(no)) {
        gravar_codigo(&dic->codigos[no->simbolo], caminho, prof);
        dic->frequencias[no->simbolo] = no->frequencia;
        dic->qtd_simbolos++;
        return;
    }

    /* No interno: desce pela esquerda com bit 0 ... */
    caminho[prof] = 0;
    percorrer(no->filhoesquerda, caminho, prof + 1, dic);

    /* ... e pela direita com bit 1 (sobrescreve a mesma posicao) */
    caminho[prof] = 1;
    percorrer(no->filhodireita, caminho, prof + 1, dic);
}

/* ========================================================================= */
/*                    ETAPA 3 - Geracao dos codigos                          */
/* ========================================================================= */

int dicionario_gerar(Dicionario *dic, const ArvoreHuffman *arv)
{
    unsigned char caminho[MAX_BITS_CODIGO];  /* bits do caminho atual */

    dicionario_zerar(dic);

    if (arv == NULL || arv->raiz == NULL)    /* arquivo vazio */
        return 0;

    percorrer(arv->raiz, caminho, 0, dic);
    return dic->qtd_simbolos;
}

/* ========================================================================= */
/*                     Consultas (usadas pela Pessoa 3)                      */
/* ========================================================================= */

int dicionario_tamanho(const Dicionario *dic, unsigned char s)
{
    return dic->codigos[s].tamanho;
}

int dicionario_bit(const Dicionario *dic, unsigned char s, int i)
{
    /* Operacao inversa de gravar_codigo: desloca o byte para a direita ate
     * o bit desejado ficar na ultima posicao e isola esse bit com & 1 */
    return (dic->codigos[s].bits[i / 8] >> (7 - (i % 8))) & 1;
}

unsigned long dicionario_total_bits(const Dicionario *dic)
{
    unsigned long total = 0;
    int s;

    for (s = 0; s < NUM_SIMBOLOS; s++)
        total += dic->frequencias[s] * (unsigned long) dic->codigos[s].tamanho;
    return total;
}

double dicionario_media_bits(const Dicionario *dic)
{
    unsigned long total_simbolos = 0;
    int s;

    for (s = 0; s < NUM_SIMBOLOS; s++)
        total_simbolos += dic->frequencias[s];

    if (total_simbolos == 0)
        return 0.0;
    return (double) dicionario_total_bits(dic) / (double) total_simbolos;
}

/* ========================================================================= */
/*                               Exibicao                                    */
/* ========================================================================= */

void dicionario_imprimir(const Dicionario *dic, FILE *saida)
{
    int s, i;

    fprintf(saida, "+---------+--------------+------+------------------------\n");
    fprintf(saida, "| Simbolo |  Frequencia  | Bits | Codigo\n");
    fprintf(saida, "+---------+--------------+------+------------------------\n");

    for (s = 0; s < NUM_SIMBOLOS; s++) {
        if (dic->codigos[s].tamanho == 0)
            continue;                        /* simbolo nao aparece */

        fprintf(saida, "| ");
        arvore_imprimir_simbolo(saida, (unsigned char) s);
        /* alinhamento: 'a' e ' ' ocupam 3 caracteres; '\n' e 0xFF ocupam 4 */
        fprintf(saida, (s >= 32 && s <= 126) ? "     " : "    ");
        fprintf(saida, "| %12lu | %4d | ", dic->frequencias[s],
                dic->codigos[s].tamanho);

        for (i = 0; i < dic->codigos[s].tamanho; i++)
            fputc('0' + dicionario_bit(dic, (unsigned char) s, i), saida);
        fputc('\n', saida);
    }
    fprintf(saida, "+---------+--------------+------+------------------------\n");
}
