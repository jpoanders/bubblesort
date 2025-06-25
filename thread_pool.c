#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "thread_pool.h"

int task_queue_init(task_queue_t* q, unsigned int size) {
    q->is_active = 1;
    q->front = 0;
    q->rear = -1;
    q->count = 0u;
    q->size = size;
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
    q->count++;
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
        printf("thread retornou null.\n");
        fflush(stdout);
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    } 
    task_t* task = &q->contents[q->front];
    q->count--;
    q->front = (q->front + 1) % q->size;
    pthread_t tid = pthread_self(); // Pega o ID da thread atual
    printf("Thread ID pegou uma task: %lu\n", (unsigned long)tid); // Conversão para exibição
    fflush(stdout);
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
    }
    pthread_exit(NULL);
}

void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args) {
    task_enqueue(&pool->task_queue, function, args);
}

int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size) {
    pool->threads = malloc(sizeof(pthread_t)*nthreads);
    task_queue_init(&pool->task_queue, q_size);

    for (unsigned int i = 0u; i < nthreads; i++) {
        if (pthread_create(&pool->threads[i], NULL, pool_worker, pool)) return 1;
    }

    pool->num_threads = nthreads;
    return 0;
}

void thread_pool_destroy(thread_pool_t* pool) {
    // desativa flag da fila
    pool->task_queue.is_active = 0;
    // acorda todas as threads
    pthread_cond_broadcast(&pool->task_queue.task_avaiable);
    // aguarda pela finalização das threads
    for (unsigned int i = 0u; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
        printf("thread %d finalizou\n", i);
        fflush(stdout);
    }
    free(pool->threads);
    task_queue_destroy(&pool->task_queue);
}