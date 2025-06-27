#include <stdlib.h>
#include "merge_sort.h"

void merge(unsigned int* v, unsigned int* aux, unsigned int ini, unsigned int meio, unsigned int fim) {
    unsigned int i = ini;
    unsigned int j = meio;
    unsigned int k = ini;

    while (i < meio && j < fim) {
        if (v[i] <= v[j]) {
            aux[k++] = v[i++];
        } else {
            aux[k++] = v[j++];
        }
    }

    while (i < meio) aux[k++] = v[i++];
    while (j < fim) aux[k++] = v[j++];

    for (i = ini; i < fim; i++) {
        v[i] = aux[i];
    }
}

void merge_sort_rec(unsigned int* v, unsigned int* aux, unsigned int ini, unsigned int fim) {
    if (fim - ini < 2) return;

    unsigned int meio = (ini + fim) / 2;
    merge_sort_rec(v, aux, ini, meio);
    merge_sort_rec(v, aux, meio, fim);
    merge(v, aux, ini, meio, fim);
}

void merge_sort(unsigned int* v, unsigned int tam) {
    if (!v || tam < 2) return;

    unsigned int* aux = (unsigned int*) malloc(tam * sizeof(unsigned int));
    if (!aux) return;

    merge_sort_rec(v, aux, 0, tam);
    free(aux);
}
