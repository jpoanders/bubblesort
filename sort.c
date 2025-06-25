#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "thread_pool.h"

typedef struct {
    int* v;
    int size;
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
    bubble_sort(bs_args->v, bs_args->size);
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
    
    int* task_vector_size = malloc(sizeof(int)*ntasks); // Armazena de facto a quantidade de números em cada vetor da task
    int** task_vector = malloc(sizeof(int*)*ntasks);  // Array para vetores com tamanho variável

    // variáveis do loop
    int lower_bound = 0;
    int upper_bound = range - 1;
    int* auxiliar_vector = malloc(sizeof(int)*tam);

    for (int j = 0; j < ntasks; j++) {
        if (remainder) {
            upper_bound++;
            remainder--;
        }
        task_vector_size[j] = 0;
        int element_index = 0;
        for (int i = 0; i < tam; i++) {
            if ((vetor[i] <= upper_bound) && (vetor[i] >= lower_bound)) {
                task_vector_size[j]++;
                auxiliar_vector[element_index] = vetor[i];
                element_index++;
            }
        } 
        // alocação do subvetor
        if (task_vector_size[j] == 0) task_vector[j] = NULL;
        else task_vector[j] = malloc(sizeof(int)*task_vector_size[j]);
        for (int i = 0; i < task_vector_size[j]; i++) {
            task_vector[j][i] = auxiliar_vector[i];
        }
        lower_bound = upper_bound + 1;
        upper_bound = upper_bound + range;
    }  
    
    free(auxiliar_vector);
    for (int i = 0; i < ntasks; i++) {
        if (task_vector[i] == NULL) printf("tentando acessar vetor NULL");
        imprime_vet((unsigned int *)task_vector[i], task_vector_size[i]);
    }
    bubble_sort_args args[ntasks];
    thread_pool_t pool;
    thread_pool_init(&pool, nthreads, ntasks);
    // adiciona as tasks à pool
    for (int i = 0; i < ntasks; i++) {
        args[i].v = task_vector[i];
        args[i].size = task_vector_size[i];
        thread_pool_add_task(&pool, bubble_sort_wrapper, &args[i]);
    }
    thread_pool_destroy(&pool);
    printf("destrui a pool.\n");
    fflush(stdout);

    for (int i = 0; i < ntasks; i++) {
        imprime_vet((unsigned int *)task_vector[i], task_vector_size[i]);
    }
    
        // concatena subvetores
    int pos = 0;
    for (int i = 0; i < ntasks; i++) {
        for (int j = 0; j < task_vector_size[i]; j++)
            vetor[pos++] = task_vector[i][j];
    }
    
    free(task_vector_size);
    for (int i = 0; i < ntasks; i++)
        free(task_vector[i]);
    free(task_vector);

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