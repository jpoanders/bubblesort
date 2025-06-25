#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "thread_pool.h"

typedef struct {
    int* vector;
    int size;
    int lower_bound, upper_bound;
} bubble_sort_args;

// Funcao de ordenacao fornecida. Não pode alterar.
void bubble_sort(int *v, int tam){
    int i, j, temp, trocou;

    for(j = 0; j < tam - 1; j++){
        trocou = 0;
        for(i = 0; i < tam - 1; i++){
            if(v[i + 1] < v[i]){
                temp = v[i];
                v[i] = v[i + 1];
                v[i + 1] = temp;
                trocou = 1;
            }
        }
        if(!trocou) break;
    }
}

void bubble_sort_wrapper(void* args) {
    bubble_sort_args* bs_args = (bubble_sort_args*)(args);
    int* auxiliar_vector = malloc(sizeof(int)*bs_args->size);
    int subvector_size = 0;
    int lower_bound = bs_args->lower_bound, upper_bound = bs_args->upper_bound;
    int* vector = bs_args->vector;
    for (int i = 0; i <= bs_args->size; i++) {
        if (lower_bound <= vector[i] && vector[i] <= upper_bound) {
            auxiliar_vector[subvector_size] = vector[i];
            subvector_size++;
        }
    }
    int* subvector = malloc(sizeof(int)*subvector_size);
    for (int i = 0; i < subvector_size; i++) {
        subvector[i] = auxiliar_vector[i];
    }
    free(auxiliar_vector);
    bs_args->size = subvector_size;
    bs_args->vector = subvector;
    bubble_sort(subvector, subvector_size);
}


// Funcao para imprimir um vetor. Não pode alterar.
void imprime_vet(unsigned int *v, int tam) {
    int i;
    for(i = 0; i < tam; i++)
        printf("%d ", v[i]);
    printf("\n");
}

// Funcao para ler os dados de um arquivo e armazenar em um vetor em memoroa. Não pode alterar.
int le_vet(char *nome_arquivo, unsigned int *v, int tam) {
    FILE *arquivo;
    
    // Abre o arquivo
    arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return 0;
    }

    // Lê os números
    for (int i = 0; i < tam; i++)
        fscanf(arquivo, "%u", &v[i]);

    fclose(arquivo);

    return 1;
}

// Funcao principal de ordenacao. Deve ser implementada com base nas informacoes fornecidas no enunciado do trabalho.
// Os numeros ordenados deverao ser armazenanos no proprio "vetor".
int sort_paralelo(unsigned int *vetor, unsigned int tam, unsigned int ntasks, unsigned int nthreads) {
    int range = tam / ntasks; // Quantidade de números representáveis em cada vetor da task
    int remainder = tam % ntasks;
    // variáveis do loop
    int lower_bound = 0;
    int upper_bound = range - 1;

    bubble_sort_args args[ntasks];
    thread_pool_t pool;
    
    thread_pool_init(&pool, nthreads, ntasks);
    for (int i = 0; i < ntasks; i++) {
        upper_bound += (i < remainder) ? 1:0;
        args[i].vector = (int*) vetor;
        args[i].size = (int) tam;
        args[i].lower_bound = lower_bound;
        args[i].upper_bound = upper_bound;
        thread_pool_add_task(&pool, bubble_sort_wrapper, &args[i]);
        lower_bound = upper_bound + 1;
        upper_bound = upper_bound + range;
    }
    thread_pool_destroy(&pool);
    
        // concatena subvetores
    int pos = 0;
    for (int i = 0; i < ntasks; i++) {
        for (int j = 0; j < args[i].size; j++)
            vetor[pos++] = args[i].vector[j];
    }
    
    for (int i = 0; i < ntasks; i++) {
        free(args[i].vector);
    }

    return 1;
}

// Funcao principal do programa. Não pode alterar.
int main(int argc, char **argv) {
    
    // Verifica argumentos de entrada
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <input> <nnumbers> <ntasks> <nthreads>\n", argv[0]);
        return 1;
    }

    // Argumentos de entrada
    unsigned int nnumbers = atoi(argv[2]);
    unsigned int ntasks = atoi(argv[3]);
    unsigned int nthreads = atoi(argv[4]);
    
    // Aloca vetor
    unsigned int *vetor = malloc(nnumbers * sizeof(unsigned int));

    // Variaveis de controle de tempo de ordenacao
    struct timeval inicio, fim;

    // Le os numeros do arquivo de entrada
    if (le_vet(argv[1], vetor, nnumbers) == 0)
        return 1;

    // Imprime vetor desordenado
    imprime_vet(vetor, nnumbers);

    // Ordena os numeros considerando ntasks e nthreads
    gettimeofday(&inicio, NULL);
    sort_paralelo(vetor, nnumbers, ntasks, nthreads);
    gettimeofday(&fim, NULL);

    // Imprime vetor ordenado
    imprime_vet(vetor, nnumbers);

    // Desaloca vetor
    free(vetor);

    // Imprime o tempo de ordenacao
    printf("Tempo: %.6f segundos\n", fim.tv_sec - inicio.tv_sec + (fim.tv_usec - inicio.tv_usec) / 1e6);
    
    return 0;
}