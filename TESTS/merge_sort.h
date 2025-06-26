#ifndef MERGE_SORT_H
#define MERGE_SORT_H

static void merge(unsigned int* v, unsigned int* aux, unsigned int ini, unsigned int meio, unsigned int fim);

static void merge_sort_rec(unsigned int* v, unsigned int* aux, unsigned int ini, unsigned int fim);

void merge_sort(unsigned int* v, unsigned int tam);

#endif