#include "thread_queue.h"
#include "uthread.h"
#include <stdlib.h>

// Thread Queue Implementation

struct thread_queue {
    struct thread **arr;
    int capacity;
    int head;
    int tail;
};

thread_queue thread_queue_create(int capacity) {
    if (capacity < 2)
        return NULL;

    thread_queue q = malloc(sizeof(thread_queue));
    if (q == NULL)
        return NULL;
    
    q->arr = (struct thread**) malloc(capacity * sizeof(struct thread*));
    if (q->arr == NULL) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->head = -1;
    q->tail = 0;
    return q;
}

void thread_queue_destroy(thread_queue q) {
    if (q == NULL)
        return;
    
    free(q->arr);
    free(q);
}

int thread_queue_enqueue(thread_queue q, struct thread *thread) {
    if (thread_queue_size(q) == q->capacity)
        return -1; // queue is full
    
    if (thread_queue_size(q) == 0) {
        q->head = 0;
        q->tail = 0;
    } else {
        q->tail = (q->tail+ 1) % q->capacity;
    }
    
    q->arr[q->tail] = thread;
    return 0;
}

struct thread* thread_queue_dequeue(thread_queue q) {
    if (thread_queue_size(q) == 0)
        return NULL; // queue is empty

    struct thread *next = q->arr[q->head];

    if (thread_queue_size(q) == 1) {
        q->head = -1;
        q->tail = 0;
    } else {
        q->head = (q->head + 1) % q->capacity;
    }

    return next;
}

struct thread* thread_queue_peek(thread_queue q) {
    if (thread_queue_size(q) == 0)
        return NULL; // queue is empty
    
    return q->arr[q->head];
}

int thread_queue_size(thread_queue q) {
    if (q->head == -1) 
        return 0;
    return (q->tail - q->head + q->capacity) % q->capacity + 1;
}

// Thread Priority Queue Implementation

struct thread_pqueue {
    struct thread **arr;
    int capacity;
    int size;
};

thread_pqueue thread_pqueue_create(int capacity) {
    if (capacity < 2)
        return NULL;

    thread_pqueue pq = malloc(sizeof(thread_pqueue));
    if (pq == NULL)
        return NULL;
    
    pq->arr = (struct thread**) malloc(capacity * sizeof(struct thread*));
    if (pq->arr == NULL) {
        free(pq);
        return NULL;
    }

    pq->capacity = capacity;
    pq->size = 0;
    return pq;
}

void thread_pqueue_destroy(thread_pqueue pq) {
    if (pq == NULL)
        return;
    
    free(pq->arr);
    free(pq);
}

static inline int left_child(int idx) {
    return 2 * idx + 1;
}

static inline int right_child(int idx) {
    return 2 * idx + 2;
}

static inline int parent(int idx) {
    return (idx - 1) / 2;
}

static inline void swap_indeces(thread_pqueue pq, int idx1, int idx2) {
    struct thread* temp = pq->arr[idx1];
    pq->arr[idx1] = pq->arr[idx2];
    pq->arr[idx2] = temp;
}

static void heapify_up(thread_pqueue pq) {
    int idx = pq->size - 1;
    while (idx > 0) {
        int parent_idx = parent(idx);
        if (pq->arr[parent_idx]->priority >= pq->arr[idx]->priority)
            break;

        swap_indeces(pq, idx, parent_idx);
        idx = parent_idx;
    }
}

static void heapify_down(thread_pqueue pq, int idx) {
    int largest = idx;
    int l = left_child(idx);
    int r = right_child(idx);
    
    if (l < pq->size && pq->arr[l]->priority > pq->arr[largest]->priority) {
        largest = l;
    }
    
    if (r < pq->size && pq->arr[r]->priority > pq->arr[largest]->priority) {
        largest = r;
    }
    
    if (largest != idx) {
        swap_indeces(pq, idx, largest);
        heapify_down(pq, largest);
    }
}

int thread_pqueue_enqueue(thread_pqueue pq, struct thread *thread) {
    if (pq->size == pq->capacity)
        return -1; // queue is full
    
    pq->arr[pq->size] = thread;
    pq->size++;

    heapify_up(pq);

    return 0;
}

struct thread* thread_pqueue_dequeue(thread_pqueue pq) {
    if (pq->size == 0)
        return NULL; // queue is empty
    
    struct thread* next = pq->arr[0];
    pq->arr[0] = pq->arr[--pq->size];

    heapify_down(pq, 0);

    return next;
}

struct thread* thread_pqueue_peek(thread_pqueue pq) {
    if (pq->size == 0)
        return NULL; // queue is empty
    
    return pq->arr[0];
}

int thread_pqueue_size(thread_pqueue pq) {
    return pq->size;
}