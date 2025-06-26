#ifndef GOLDEN_MODEL_H
#define GOLDEN_MODEL_H

typedef struct {
    void (*sort)(unsigned int*, unsigned int);
    unsigned int* vector;
    unsigned int size;
} gm_args;

void* golden_model(void*);

#endif