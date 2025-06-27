#ifndef SORT_H
#define SORT_H

#include <pthread.h>

// Constante padrão para o tamanho da fila de tarefas
#define DEFAULT_QUEUE_SIZE 16

// Tipo de tarefa
typedef struct {
    void (*function)(void*);
    void* arg;
} task_t;

// Fila de tarefas
typedef struct {
    int front;
    int rear;
    unsigned int count;
    unsigned int size;
    task_t* contents;
    pthread_mutex_t mutex;
    pthread_cond_t task_avaiable;
    int is_active;
} task_queue_t;

// Estrutura do pool de threads
typedef struct {
    pthread_t* threads;
    unsigned int num_threads;
    task_queue_t task_queue;
} thread_pool_t;

// Argumentos para bubble_sort_wrapper
typedef struct {
    int* vector;
    int size;
    int lower_bound, upper_bound;
} bubble_sort_args;

// Funções da fila de tarefas
int task_queue_init(task_queue_t* q, unsigned int size);
void task_queue_destroy(task_queue_t* q);
int task_enqueue(task_queue_t* q, void (*function)(void*), void* arg);
task_t* task_dequeue(task_queue_t* q);

// Funções do pool de threads
void* pool_worker(void* args);
void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args);
int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size);
void thread_pool_destroy(thread_pool_t* pool);

// Algoritmos de ordenação
void bubble_sort(int* v, int tam);
void bubble_sort_wrapper(void* args);

// Funções utilitárias
void imprime_vet(unsigned int* v, int tam);
int le_vet(char* nome_arquivo, unsigned int* v, int tam);

// Função principal de ordenação paralela
int sort_paralelo(unsigned int* vetor, unsigned int tam, unsigned int ntasks, unsigned int nthreads);

#endif // SORT_H
