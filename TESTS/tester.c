#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "golden_model/golden_model.h"
#include "merge_sort.h"

int fd[2];

int sort_instance(const int nnumbers, const int ntasks, const int nthreads);
int compare_vectors(const unsigned int* v1, const unsigned int* v2, unsigned int tam);
int generator(const int min, const int max);


int main() {

    return 0;
}

int sort_instance(const int nnumbers, const int ntasks, const int nthreads) {
    char str[20][3];
    sprintf(str[0], "%d", nnumbers);
    sprintf(str[1], "%d", ntasks);
    sprintf(str[2], "%d", nthreads);
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"./sort", "teste.txt", str[0], str[1], str[2], NULL};
        execvp(args[0], args);
    } else {
        wait(NULL);  // espera o filho terminar
    }

    int* result = malloc(sizeof(int)*nnumbers);
    read(fd[0], &result, sizeof(int)*nnumbers);

    return 0;
}

int compare_vectors(const unsigned int* v1, const unsigned int* v2, unsigned int tam) {
    for (unsigned int i = 0; i < tam; i++) {
        if (v1[i] != v2[i]) {
            return 0;
        }
    }
    return 1;
}

int generator(const int min, const int max) {
    srand(time(NULL));

    // Gera valor aleatório de n
    int n = min + rand() % (max - min + 1);

    // Abre arquivo
    FILE *fp = fopen("dados.txt", "w");
    if (fp == NULL) {
        perror("Erro ao abrir dados.txt");
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
    printf("Foram gerados %d números aleatórios no arquivo dados.txt.\n", n);

    return n;
}