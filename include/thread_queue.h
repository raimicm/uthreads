#ifndef THREADQUEUE_H 
#define THREADQUEUE_H 

#include "uthread.h"

// Thread Queue

typedef struct thread_queue *thread_queue;

thread_queue thread_queue_create(int capacity);
void thread_queue_destroy(thread_queue q);
int thread_queue_enqueue(thread_queue q, struct thread *thread);
struct thread* thread_queue_dequeue(thread_queue q);
struct thread* thread_queue_peek(thread_queue q);
int thread_queue_size(thread_queue q);

// Thread Priority Queue

typedef struct thread_pqueue *thread_pqueue;

thread_pqueue thread_pqueue_create(int capacity);
void thread_pqueue_destroy(thread_pqueue pq);
int thread_pqueue_enqueue(thread_pqueue pq, struct thread *thread);
struct thread* thread_pqueue_dequeue(thread_pqueue pq);
struct thread* thread_pqueue_peek(thread_pqueue pq);
int thread_pqueue_size(thread_pqueue pq);

#endif