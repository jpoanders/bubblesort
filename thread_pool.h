#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#define DEFAULT_QUEUE_SIZE 16

typedef struct {
    void (*function)(void*);
    void* arg;
} task_t;

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

typedef struct {
    pthread_t* threads;
    unsigned int num_threads;
    task_queue_t task_queue;
} thread_pool_t;


int task_queue_init(task_queue_t* q, unsigned int size);

void task_queue_destroy(task_queue_t* q);

int task_enqueue(task_queue_t* q, void (*function)(void*), void* arg);

task_t* task_dequeue(task_queue_t* q);

void* pool_worker(void* args);

void thread_pool_add_task(thread_pool_t* pool, void (*function)(void*), void* args);

int thread_pool_init(thread_pool_t* pool, unsigned int nthreads, unsigned int q_size);

void thread_pool_destroy(thread_pool_t* pool);

#endif