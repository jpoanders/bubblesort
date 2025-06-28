#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#define DEFAULT_QUEUE_SIZE 16

typedef struct {
    void (*function)(void*);
    void* arg;
    unsigned int id;
} task_t;

typedef struct {
    int front;
    int rear;
    unsigned int count;
    unsigned int size;
    unsigned int total_enqueued;
    task_t* contents;
    pthread_mutex_t mutex;
    pthread_cond_t task_avaiable;
    int is_active;
} task_queue_t;

typedef struct {
    pthread_t* threads;
    unsigned int num_threads;
    task_queue_t task_queue;
} thread_pool_t;

typedef struct {
    int* vector;
    int size;
    int lower_bound, upper_bound;
} bubble_sort_args;


int task_queue_init(task_queue_t* q, unsigned int size);

void task_queue_destroy(task_queue_t* q);

int task_enqueue(task_queue_t* q, void (*function)(void*), void* arg);

task_t* task_dequeue(task_queue_t* q);

void* pool_worker(void* args);

void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args);

int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size);

void thread_pool_destroy(thread_pool_t* pool);

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
    for (int i = 0; i < bs_args->size; i++) {
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


// IMPLEMENTAÇÃO DA THREAD POOL E TASK QUEUE

int task_queue_init(task_queue_t* q, unsigned int size) {
    q->is_active = 1;
    q->front = 0;
    q->rear = -1;
    q->count = 0u;
    q->size = size;
    q->total_enqueued = 0u;
    unsigned int queue_size = (size) ? size : DEFAULT_QUEUE_SIZE;
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
    while (q->count == q->size) {
        pthread_cond_wait(&q->task_avaiable, &q->mutex);
    }
    q->rear = (q->rear + 1) % q->size;
    q->contents[q->rear].function = function;
    q->contents[q->rear].arg = arg;
    q->contents[q->rear].id = q->total_enqueued;
    q->count++;
    q->total_enqueued++;
    pthread_cond_broadcast(&q->task_avaiable);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

task_t* task_dequeue(task_queue_t* q) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0u && q->is_active) {
        pthread_t tid = pthread_self(); // Pega o ID da thread atual
        printf("Thread ID está esperando: %lu\n", (unsigned long)tid); // Conversão para exibição
        fflush(stdout);
        pthread_cond_wait(&q->task_avaiable, &q->mutex);
    }
    if (q->count == 0u && !q->is_active) {
        pthread_cond_broadcast(&q->task_avaiable);
        // printf("thread retornou null.\n");
        // fflush(stdout);
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    } 
    task_t* task = &q->contents[q->front];
    q->count--;
    q->front = (q->front + 1) % q->size;
    pthread_cond_broadcast(&q->task_avaiable);
    pthread_mutex_unlock(&q->mutex);
    return task;
}

void* pool_worker(void* args) {
    thread_pool_t* pool = (thread_pool_t*) args;

    while (1) {
        task_t* task = task_dequeue(&pool->task_queue);
        if (task == NULL) break;
        task->function(task->arg);
        pthread_t tid = pthread_self();
        printf("Thread %lu pegou a task %u.\n", (unsigned long)tid, task->id);
        fflush(stdout);
    }
    pthread_exit(NULL);
}

void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args) {
    task_enqueue(&pool->task_queue, function, args);
}

int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size) {
    pool->threads = malloc(sizeof(pthread_t)*nthreads);
    pool->num_threads = nthreads;
    task_queue_init(&pool->task_queue, q_size);
    for (unsigned int i = 0u; i < nthreads; i++) {
        if (pthread_create(&pool->threads[i], NULL, pool_worker, pool)) return 1;
    }
    return 0;
}

void thread_pool_destroy(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->task_queue.mutex);
    pool->task_queue.is_active = 0;
    pthread_cond_broadcast(&pool->task_queue.task_avaiable);
    pthread_mutex_unlock(&pool->task_queue.mutex);
    for (unsigned int i = 0u; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
        printf("Uma Thread finalizou\n");
        fflush(stdout);
    }
    printf("Todas as Threads finalizaram\n");
    free(pool->threads);
    task_queue_destroy(&pool->task_queue);
}
