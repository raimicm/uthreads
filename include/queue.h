#ifndef QUEUE_H
#define QUEUE_H

// Queue

typedef struct queue *queue;

queue queue_create(int capacity);
void queue_destroy(queue q);
int queue_enqueue(queue q, int val);
int queue_dequeue(queue q);
int queue_peek(queue q);
int queue_size(queue q);

// Priority Queue

typedef struct priority_queue *priority_queue;

priority_queue priority_queue_create(int capacity);
void priority_queue_destroy(priority_queue pq);
int priority_queue_enqueue(priority_queue pq, int val, int priority);
int priority_queue_dequeue(priority_queue pq);
int priority_queue_peek(priority_queue pq);
int priority_queue_size(priority_queue pq);

#endif