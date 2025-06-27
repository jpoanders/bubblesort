#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "merge_sort.h"
#include "sort.h"

typedef struct {
    unsigned int* unordered_vector;
    unsigned int* result_vector;
} sort_instance_ret;

sort_instance_ret* sort_instance(const int nnumbers, const int ntasks, const int nthreads);
int generator(const int min, const int max);
int random_number(const int min, const int max);
int compare_vectors(unsigned int* v1, unsigned int* v2, int size);

int main(int argc, char** argv) {
    if (argc != 6) {
        printf("./tester <min_len> <max_len> <max_tasks> <max_threads> <instances>\n");
        return 1;
    }

    int min = atoi(argv[1]), max = atoi(argv[2]);
    int max_tasks = atoi(argv[3]);
    int max_threads = atoi(argv[4]);
    int number_instances = atoi(argv[5]);
    int ntasks, nthreads;
    int count_correct = 0;

    for (int i = 0; i < number_instances; i++) {
        ntasks = random_number(1, max_tasks);
        nthreads = random_number(1, max_threads);
        int nnumbers = generator(min, max);
        sort_instance_ret* ret_value = sort_instance(nnumbers, ntasks, nthreads);
        printf("Instancia com %d tasks e %d threads finalizou.\n", ntasks, nthreads);
        merge_sort(ret_value->unordered_vector, nnumbers);
        if (compare_vectors(ret_value->unordered_vector, ret_value->result_vector, nnumbers)) {
            printf("Ordenamento correto.\n");
            fflush(stdout);
            count_correct++;
        } else {
            printf("Resultado Incorreto.\n");
            fflush(stdout);
        }
        free(ret_value->result_vector);
        free(ret_value->unordered_vector);
        free(ret_value);
        sleep(1);
    }
    printf("Testes finalizados.\n");
    printf("%d/%d testes passados.\n", count_correct, number_instances);

    return 0;
}

sort_instance_ret* sort_instance(const int nnumbers, const int ntasks, const int nthreads) {
    // char str[3][20];
    // sprintf(str[0], "%d", nnumbers);
    // sprintf(str[1], "%d", ntasks);
    // sprintf(str[2], "%d", nthreads);
    // printf("%s %s %s", str[0], str[1], str[2]);
    // fflush(stdout);
    // pid_t pid = fork();
    // if (pid == 0) {
    //     char *args[] = {"./sort", "teste.txt", str[0], str[1], str[2], NULL};
    //     execvp(args[0], args);
    //     exit(0);
    // } else {
    //     wait(NULL);  // espera o filho terminar
    // }
       // Aloca vetor
    unsigned int *vetor = malloc(nnumbers * sizeof(unsigned int));

    // Variaveis de controle de tempo de ordenacao
    struct timeval inicio, fim;

    // Le os numeros do arquivo de entrada
    if (le_vet("teste.txt", vetor, nnumbers) == 0)
        return NULL;

    // Imprime vetor desordenado
    // imprime_vet(vetor, nnumbers);

    // Copia vetor
    unsigned int* unordered_vector = malloc(nnumbers * sizeof(unsigned int));
    for (int i = 0; i < nnumbers; i++) {
        unordered_vector[i] = vetor[i];
    }

    // Ordena os numeros considerando ntasks e nthreads
    gettimeofday(&inicio, NULL);
    sort_paralelo(vetor, nnumbers, ntasks, nthreads);
    gettimeofday(&fim, NULL);

    // Imprime vetor ordenado
    // imprime_vet(vetor, nnumbers);

    // Desaloca vetor
    // free(vetor);

    // Imprime o tempo de ordenacao
    printf("Tempo: %.6f segundos\n", fim.tv_sec - inicio.tv_sec + (fim.tv_usec - inicio.tv_usec) / 1e6);
    
    sort_instance_ret* ret_value = malloc(sizeof(sort_instance_ret));
    ret_value->unordered_vector = unordered_vector;
    ret_value->result_vector = vetor;
    return ret_value;
}

int generator(const int min, const int max) {
    srand(time(NULL));

    // Gera valor aleatório de n
    int n = min + rand() % (max - min + 1);

    // Abre arquivo
    FILE *fp = fopen("teste.txt", "w");
    if (fp == NULL) {
        perror("Erro ao abrir teste.txt");
        return 1;
    }

    // Gera e escreve n números aleatórios entre 0 e n-1 (com possíveis repetições)
    for (int i = 0; i < n; i++) {
        int valor = rand() % n;
        fprintf(fp, "%d", valor);
        if (i < n - 1) {
            fprintf(fp, " ");
        }
    }

    fclose(fp);

    // Imprime no terminal
    printf("Foram gerados %d números aleatórios no arquivo teste.txt.\n", n);

    return n;
}

int random_number(const int min, const int max) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)(clock() + getpid()));  // Ou time(NULL)
        seeded = 1;
    }

    int r = rand();
    return min + (r % (max - min + 1));
}

int compare_vectors(unsigned int* v1, unsigned int* v2, int size) {
    for (int i = 0; i < size; i++) {
        if (v1[i] != v2[i]) {
            return 0; // São diferentes
        }
    }
    return 1; // São iguais
}
