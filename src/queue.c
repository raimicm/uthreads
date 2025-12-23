#include "queue.h"
#include <stdlib.h>

struct queue {
    int *arr;
    int capacity;
    int head;
    int tail;
};

queue* queue_create(int capacity) {
    if (capacity < 2)
        return NULL;

    queue *q = malloc(sizeof(queue));
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

void queue_destroy(queue *q) {
    if (q == NULL)
        return;
    
    free(q->arr);
    free(q);
}

int queue_enqueue(queue *q, int data) {
    if (queue_size(q) == q->capacity)
        return -1; // queue is full
    
    if (queue_size(q) == 0) {
        q->head = 0;
        q->tail = 0;
    } else {
        q->tail = (q->tail+ 1) % q->capacity;
    }
    
    q->arr[q->tail] = data;
    return 0;
}

int queue_dequeue(queue *q) {
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

int queue_peek(queue *q) {
    if (queue_size(q) == 0)
        return -1; // queue is empty
    
    return q->arr[q->head];
}

int queue_size(queue *q) {
    if (q->head == -1) 
        return 0;
    return (q->tail - q->head + q->capacity) % q->capacity + 1;
}