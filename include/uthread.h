#ifndef UTHREAD_H 
#define UTHREAD_H    

// User threads

/*
 * Scheduling Policies:
 *   1. FIFO
 *   2. Priority
 *   3. Round Robin
 *   4. Complemtely Fair Scheduler
 *   5. Multi-Level Feedback Queue
 */

#include <stdlib.h>
#include <stdbool.h>

#define MAX_THREADS 64
#define DEFAULT_STACK_SIZE (64 * 1024)

typedef int uthread;

struct thread {
    void* stack_end;
    void* sp;
    void* (*func)(void*);
    void* args;
    void* retval;
    bool terminated;
    uthread join_id;
};


typedef enum {
    FIFO, // First-In-First-Out
    PS,    // Priority Scheduling 
    RR,   // Round Robin
    CFS,   // Completely Fair Scheduler
    MLFQ  // Multi-Level Feedback Queue
} sched_policy;


void uthread_init(sched_policy policy, size_t stack_sz);
int uthread_create(uthread *thread, void* (*func)(void*), void *args);
int uthread_join(uthread thread, void **retval);
void uthread_exit(void *retval);

#endif