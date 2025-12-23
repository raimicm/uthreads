#ifndef QUEUE_H
#define QUEUE_H

typedef struct queue queue;

queue* queue_create(int capacity);
void queue_destroy(queue* q);
int queue_enqueue(queue* q, int data);
int queue_dequeue(queue* q);
int queue_peek(queue* q);
int queue_size(queue* q);

#endif