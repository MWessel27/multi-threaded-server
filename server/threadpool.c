/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

typedef struct node_worker{
	void (*function) (void*);
  struct node_worker* next;
	void * arguments;
} node_worker_t;

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
   // you should fill in this structure with whatever you need
   int thread_count; // current thread count
   int queue_count;  // number of threads in queue
   pthread_mutex_t   mutex;   // lock
   pthread_cond_t    condvar; // conditional variable for empty
   pthread_t         *threads // thread pointer in pool
   node_worker_t     *head;   // pointer to head of queue
   node_worker_t     *end;    // pointer to end of queue
   int shutdown;
} _threadpool;

void *thread_main(threadpool inpool) {
  _threadpool * pool = (_threadpool *) inpool;

  node_worker_t *this;

  while(1) {
    pool->queue_count = pool->queue_count;
    // grab the mutex
    if (pthread_mutex_lock(&mutex) != 0) {
      perror("mutex lock failed (!!):");
      exit(-1);
    }

    while((pool->queue_count == 0) && (!pool->shutdown)) {
      pthread_cond_wait(&(pool->condvar),&(pool->mutex));
    }

    if(pool->shutdown) {
      pthread_mutex_unlock(&(pool->mutex));
      pthread_exit(NULL);
    }

    this = pool->head;
    pool->queue_count--;

    if(!pool->queue_count) {
      pool->head = NULL;
      pool->end = NULL;
    } else {
      pool->head = this->next;
    }

    if(pool->queue_count == 0) && (!pool->shutdown) {
      pthread_cond_signal(&(pool->condvar));
    }

    pthread_mutex_unlock(&(pool->mutex));
    (this->function) (cur->arguments);
    free(this);
  }
}

threadpool create_threadpool(int num_threads_in_pool) {
  _threadpool *pool;

  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

  // add your code here to initialize the newly created threadpool
  pool->threads = (pthread_t*)malloc(num_threads_in_pool * sizeof(pthread_t));

  if(pool->threads == NULL){
    fprintf(stderr, "Out of memory creating threadpool threads!\n");
    return NULL;
  }

  // since we have our pool and threads, lets set up our struct
  pool->thread_count = num_threads_in_pool;
  pool->head = pool->end = NULL;
  pool->queue_count = pool->shutdown = 0;

  // initialize the mutex and condition variable
  pthread_mutex_init(&(pool->mutex), NULL);
  pthread_cond_init(&(pool->condvar), NULL);

  // create threads
  for(int i=0;i<num_threads_in_pool;i++){
    if(pthread_create(&(pool->threads[i]),NULL,do_work,pool) != 0){
      perror("Thread creation failed:");
      exit(-1);
    }
  }
  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg) {
  _threadpool *pool = (_threadpool *) from_me;
  node_worker_t *this;

  // add your code here to dispatch a thread
  this = (node_worker_t*)malloc(sizeof(node_worker_t));
  if(this == NULL){
    fprintf(stderr, "Out of memory creating threadpool threads!\n");
    return NULL;
  }

  this->function   = dispatch_to_here;
  this->arguments = arg;
  this->next      = NULL;

  pthread_mutex_lock(&(pool->mutex));

  if(pool->queue_count == 0) {
    pool->head = this;
    pool->end  = this;
    pthread_cond_signal(&(pool->condvar));
  } else {
    pool->end->next = this;
    pool->end       = this;
  }

  pool->queue_count++;
  pthread_mutex_unlock(&(pool->mutex));
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  // add your code here to kill a threadpool
  if (pthread_mutex_lock(&mutex) != 0) {
    perror("mutex lock failed (!!):");
    exit(-1);
  }

  pool->shutdown = 1;
  pthread_cond_broadcast(&(pool->condvar));
  pthread_mutex_unlock(&(pool->mutex));
  for(i = 0; i < pool->thread_count; i++) {
    pthread_join(pool->threads[i], NULL);
  }

  free(pool->threads);
  return;
}

