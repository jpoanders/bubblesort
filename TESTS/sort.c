#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#define DEFAULT_QUEUE_SIZE 16

#include <pthread.h>

/**
 * @brief Representa uma tarefa a ser executada por uma thread.
 */
typedef struct {
    void (*function)(void*);  ///< Ponteiro para a função da tarefa.
    void* arg;                ///< Argumento para a função.
    unsigned int id;          ///< ID da tarefa (opcional para rastreamento).
} task_t;

/**
 * @brief Fila circular de tarefas com controle de concorrência.
 */
typedef struct {
    int front;                         ///< Índice do primeiro elemento da fila.
    int rear;                          ///< Índice do último elemento da fila.
    unsigned int count;                ///< Número de tarefas atualmente na fila.
    unsigned int size;                 ///< Capacidade máxima da fila.
    unsigned int total_enqueued;       ///< Contador total de tarefas inseridas.
    task_t* contents;                  ///< Vetor de tarefas.
    pthread_mutex_t mutex;             ///< Mutex para sincronização de acesso.
    pthread_cond_t task_avaiable;      ///< Condição para sinalizar disponibilidade de tarefa.
    int is_active;                     ///< Indicador de atividade da fila (1 = ativa, 0 = inativa).
} task_queue_t;

/**
 * @brief Dados passados para cada thread da pool.
 */
typedef struct {
    task_queue_t* task_queue;  ///< Ponteiro para a fila de tarefas compartilhada.
    unsigned int id;           ///< ID da thread.
} thread_data;

/**
 * @brief Estrutura principal da thread pool.
 */
typedef struct {
    pthread_t* threads;            ///< Vetor de identificadores de threads.
    unsigned int num_threads;     ///< Número total de threads.
    task_queue_t task_queue;      ///< Fila de tarefas da pool.
    thread_data* threads_data;    ///< Dados específicos para cada thread.
} thread_pool_t;

/**
 * @brief Argumentos para a função de ordenação com bubble sort.
 */
typedef struct {
    int* vector;             ///< Vetor inteiro original (pré-ordenação). Subvetor ordenado (pós-ordenação).
    int size;                ///< Tamanho total do vetor (pré-ordenação). Tamanho do subvetor a ordenar/ordenado.
    int lower_bound;         ///< Índice inicial da subparte a ordenar.
    int upper_bound;         ///< Índice final (inclusivo) da subparte a ordenar.
} bubble_sort_args;

/**
 * @brief Inicializa a fila de tarefas.
 *
 * @param q Ponteiro para a fila.
 * @param size Capacidade máxima da fila.
 * @return 0 em sucesso, -1 em caso de erro.
 */
int task_queue_init(task_queue_t* q, unsigned int size);

/**
 * @brief Libera recursos da fila de tarefas.
 *
 * @param q Ponteiro para a fila.
 */
void task_queue_destroy(task_queue_t* q);

/**
 * @brief Enfileira uma nova tarefa.
 *
 * @param q Ponteiro para a fila.
 * @param function Ponteiro para a função da tarefa.
 * @param arg Argumento a ser passado para a função.
 * @return 0 em sucesso, -1 em caso de erro ou fila cheia.
 */
int task_enqueue(task_queue_t* q, void (*function)(void*), void* arg);

/**
 * @brief Remove uma tarefa da fila.
 *
 * @param q Ponteiro para a fila.
 * @return Ponteiro para a tarefa, ou NULL se vazia e inativa.
 */
task_t* task_dequeue(task_queue_t* q);

/**
 * @brief Função executada por cada thread da pool.
 *
 * @param args Ponteiro para dados da thread.
 * @return Sempre NULL.
 */
void* pool_worker(void* args);

/**
 * @brief Adiciona uma tarefa à thread pool.
 *
 * @param pool Ponteiro para a thread pool.
 * @param function Função a ser executada.
 * @param args Argumento para a função.
 */
void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args);

/**
 * @brief Inicializa a thread pool.
 *
 * @param pool Ponteiro para a estrutura da pool.
 * @param nthreads Número de threads.
 * @param q_size Tamanho máximo da fila de tarefas.
 * @return 0 em sucesso, -1 em erro.
 */
int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size);

/**
 * @brief Destroi a thread pool e seus recursos.
 *
 * @param pool Ponteiro para a thread pool.
 */
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

/**
 * @brief Função que calcula o subvetor a ordenar e que envelopa bubble_sort.
 * 
 * @param args Argumento advindo de determinada tarefa.
 */
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
    // Tarefas com intervalos que não possuem valores não vão para a ETAPA 2
    if (!subvector_size) {
        free(auxiliar_vector);
        bs_args->size = 0;
        bs_args->vector = NULL;
        return;
    }
    int* subvector = malloc(sizeof(int)*subvector_size);
    for (int i = 0; i < subvector_size; i++) {
        subvector[i] = auxiliar_vector[i];
    }
    free(auxiliar_vector);
    bs_args->size = subvector_size;
    bs_args->vector = subvector;
    // Fim da ETAPA 1 e início da ETAPA 2 para determinada tarefa
    bubble_sort(subvector, subvector_size);
    // Fim da ETAPA 2 para determinada tarefa
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
    int range = tam / ntasks; // Quantidade de números representáveis em cada vetor da tarefa
    int remainder = tam % ntasks;
    // Variáveis de tarefa
    int lower_bound = 0;
    int upper_bound = range - 1;

    bubble_sort_args args[ntasks];
    thread_pool_t pool;
    
    thread_pool_init(&pool, nthreads, ntasks);
    // Início da ETAPA 1: divisão em tarefas
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
    
    // ETAPA 3: concatenação dos subvetores
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

/**
 * 
 *  IMPLEMENTAÇÃO DAS FUNÇÕES RELATIVAS À THREAD POOL E À TASK QUEUE
 * 
 */

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
        pthread_cond_wait(&q->task_avaiable, &q->mutex);
    }
    if (q->count == 0u && !q->is_active) {
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
    thread_data* data = (thread_data*) args;

    while (1) {
        task_t* task = task_dequeue(data->task_queue);
        if (task == NULL) break;
        printf("Thread %u processando tarefa %u.\n", data->id, task->id);
        fflush(stdout);
        task->function(task->arg);
    }
    pthread_exit(NULL);
}

void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args) {
    task_enqueue(&pool->task_queue, function, args);
}

int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size) {
    pool->threads = malloc(sizeof(pthread_t)*nthreads);
    pool->threads_data = malloc(sizeof(thread_data)*nthreads);
    pool->num_threads = nthreads;
    task_queue_init(&pool->task_queue, q_size);
    for (unsigned int i = 0u; i < nthreads; i++) {
        pool->threads_data[i].id = i;
        pool->threads_data[i].task_queue = &pool->task_queue;
        if (pthread_create(&pool->threads[i], NULL, pool_worker, &pool->threads_data[i])) return 1;
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
        printf("Thread %u finalizou.\n", i);
        fflush(stdout);
    }
    printf("Todas as Threads finalizaram\n");
    free(pool->threads);
    free(pool->threads_data);
    task_queue_destroy(&pool->task_queue);
}