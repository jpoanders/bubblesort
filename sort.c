#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#define DEFAULT_QUEUE_SIZE 16

typedef struct {
    int* v;
    int size;
} bubble_sort_args;


typedef struct {
    void (*function)(void*);
    void* arg;
} task_t;

typedef struct {
    int front;
    int rear;
    size_t count;
    size_t size;
    task_t* contents;
    pthread_mutex_t mutex;
    pthread_cond_t task_avaiable;
} task_queue_t;

int task_queue_init(task_queue_t* q, size_t size) {
    q->front = 0;
    q->rear = -1;
    q->count = 0u;
    q->size = size;
    size_t queue_size = (size) ? size : DEFAULT_QUEUE_SIZE;
    task_t* contents = malloc(sizeof(task_t)*queue_size);
    q->contents = contents;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->task_avaiable, NULL);
    return 0;
}

void task_queue_destroy(task_queue_t* q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->task_avaiable);
    free(q->contents);
}

int task_enqueue(task_queue_t* q, void (*function)(void*), void* arg) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == q->size)
        pthread_cond_wait(&q->task_avaiable, &q->mutex);
    q->rear = (q->rear + 1) % q->size;
    q->contents[q->rear].function = function;
    q->contents[q->rear].arg = arg;
    pthread_cond_signal(&q->task_avaiable);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

task_t* task_dequeue(task_queue_t* q) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0u)
        pthread_cond_wait(&q->task_avaiable, &q->mutex);
    task_t* task = &q->contents[q->front];
    q->count--;
    q->front = (q->front + 1) % q->size;
    pthread_cond_signal(&q->task_avaiable);
    pthread_mutex_unlock(&q->mutex);
    return task;
}

void* pool_worker(void* args) {
    task_queue_t* task_queue = (task_queue_t*) args;

    while (1) {
        task_t* task = task_dequeue(task_queue);
        task->function(task->arg);
    }
}

typedef struct {
    pthread_t* threads;
    size_t num_threads;
    task_queue_t task_queue;
} thread_pool_t;


void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args) {
    task_enqueue(&pool->task_queue, function, args);
}

int thread_pool_init(thread_pool_t* pool, size_t nthreads, size_t q_size) {
    pool->threads = malloc(sizeof(pthread_t)*nthreads);
    task_queue_init(&pool->task_queue, q_size);

    for (size_t i = 0u; i < nthreads; i++) {
        if (pthread_create(&pool->threads[i], NULL, pool_worker, &pool->task_queue)) return 1;
    }

    pool->num_threads = nthreads;
    return 0;
}

void thread_pool_destroy(thread_pool_t* pool) {
    for (size_t i = 0u; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    free(pool->threads);
    task_queue_destroy(&pool->task_queue);
}

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
    int vector_index = 0;  

    while (upper_bound < tam) {
        if (remainder) {
            upper_bound++;
            remainder--;
        }
        task_vector_size[vector_index] = 0;
        // task_vector[vector_index] = malloc(sizeof(int) * ???)
        // implementar vetor dinâmico
        int element_index = 0;
        for (int i = 0; i < tam; i++) {
            if ((vetor[i] <= upper_bound) && (vetor[i] >= lower_bound)) {
                task_vector_size[vector_index]++;
                task_vector[vector_index][element_index] = vetor[i];
                element_index++;
            }
        } 
        // controle
        lower_bound = upper_bound + 1;
        upper_bound = upper_bound + range;
        vector_index++;
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

    // concatena subvetores
    int pos = 0;
    for (int i = 0; i < tam; i++) {
        for (int j = pos; j < task_vector_size[i]; j++)
            vetor[pos++] = task_vector[i][j];
    }

    thread_pool_destroy(&pool);
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