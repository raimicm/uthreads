#include "queue.h"
#include <stdlib.h>

// Queue Implementation

struct queue {
    int *arr;
    int capacity;
    int head;
    int tail;
};

queue queue_create(int capacity) {
    if (capacity < 2)
        return NULL;

    queue q = malloc(sizeof(queue));
    if (q == NULL)
        return NULL;
    
    q->arr = (int*) malloc(capacity * sizeof(int));
    if (q->arr == NULL) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->head = -1;
    q->tail = 0;
    return q;
}

void queue_destroy(queue q) {
    if (q == NULL)
        return;
    
    free(q->arr);
    free(q);
}

int queue_enqueue(queue q, int val) {
    if (queue_size(q) == q->capacity)
        return -1; // queue is full
    
    if (queue_size(q) == 0) {
        q->head = 0;
        q->tail = 0;
    } else {
        q->tail = (q->tail+ 1) % q->capacity;
    }
    
    q->arr[q->tail] = val;
    return 0;
}

int queue_dequeue(queue q) {
    if (queue_size(q) == 0)
        return -1; // queue is empty

    int result = q->arr[q->head];

    if (queue_size(q) == 1) {
        q->head = -1;
        q->tail = 0;
    } else {
        q->head = (q->head + 1) % q->capacity;
    }

    return result;
}

int queue_peek(queue q) {
    if (queue_size(q) == 0)
        return -1; // queue is empty
    
    return q->arr[q->head];
}

int queue_size(queue q) {
    if (q->head == -1) 
        return 0;
    return (q->tail - q->head + q->capacity) % q->capacity + 1;
}

// Priority Queue Implementation

struct data {
    int val;
    int priority;
};

struct priority_queue {
    struct data *arr;
    int capacity;
    int size;
};

priority_queue priority_queue_create(int capacity) {
    if (capacity < 2)
        return NULL;

    priority_queue pq = malloc(sizeof(priority_queue));
    if (pq == NULL)
        return NULL;
    
    pq->arr = (struct data*) malloc(capacity * sizeof(struct data));
    if (pq->arr == NULL) {
        free(pq);
        return NULL;
    }

    pq->capacity = capacity;
    pq->size = 0;
    return pq;
}

void priority_queue_destroy(priority_queue pq) {
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

static inline void swap_indeces(priority_queue pq, int idx1, int idx2) {
    struct data temp = pq->arr[idx1];
    pq->arr[idx1] = pq->arr[idx2];
    pq->arr[idx2] = temp;
}

static void heapify_up(priority_queue pq) {
    int idx = pq->size - 1;
    while (idx > 0) {
        int parent_idx = parent(idx);
        if (pq->arr[parent_idx].priority > pq->arr[idx].priority)
            break;

        swap_indeces(pq, idx, parent_idx);
        idx = parent_idx;
    }
}

static void heapify_down(priority_queue pq, int idx) {
    int largest = idx;
    int l = left_child(idx);
    int r = right_child(idx);
    
    if (l < pq->size && pq->arr[l].priority > pq->arr[largest].priority) {
        largest = l;
    }
    
    if (r < pq->size && pq->arr[r].priority > pq->arr[largest].priority) {
        largest = r;
    }
    
    if (largest != idx) {
        swap_indeces(pq, idx, largest);
        heapify_down(pq, largest);
    }
}

int priority_queue_enqueue(priority_queue pq, int val, int priority) {
    if (pq->size == pq->capacity)
        return -1; // priority queue is full
    
    pq->arr[pq->size].val = val;
    pq->arr[pq->size].priority = priority;
    pq->size++;

    heapify_up(pq);

    return 0;
}

int priority_queue_dequeue(priority_queue pq) {
    if (pq->size == 0)
        return -1; // priority queue is empty
    
    int result = pq->arr[0].val;
    pq->arr[0] = pq->arr[--pq->size];

    heapify_down(pq, 0);

    return result;
}

int priority_queue_peek(priority_queue pq) {
    if (pq->size == 0)
        return -1; // priority_queue is empty
    
    return pq->arr[0].val;
}

int priority_queue_size(priority_queue pq) {
    return pq->size;
}