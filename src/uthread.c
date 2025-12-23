#include "uthread.h"
#include "context_switch.h"
#include "queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

bool initialized = false;
sched_policy scheduling_policy = FIFO;
size_t stack_size = DEFAULT_STACK_SIZE;

struct thread *threads[MAX_THREADS];
unsigned last_id = 0;
unsigned thread_count = 0;

uthread curthread_id;
queue *runqueue;

void thread_execute() {
    struct thread *curthread = threads[curthread_id];

    // call func(args)
    void* retval = curthread->func(curthread->args);

    // thread finished
    uthread_exit(retval);
}

static void* thread_setup_stack(void *stack_bottom) {
    // push context to stack 
    uint64_t *sp = (uint64_t*) stack_bottom;
    *(--sp) = (uint64_t) thread_execute; // return address 
    *(--sp) = 0;  // rbp
    *(--sp) = 0;  // r15
    *(--sp) = 0;  // r14
    *(--sp) = 0;  // r13
    *(--sp) = 0;  // r12
    *(--sp) = 0;  // r11
    *(--sp) = 0;  // r10
    *(--sp) = 0;  // r9
    *(--sp) = 0;  // r8
    *(--sp) = 0;  // rdi
    *(--sp) = 0;  // rsi
    *(--sp) = 0;  // rdx
    *(--sp) = 0;  // rcx
    *(--sp) = 0;  // rbx
    *(--sp) = 0;  // rax

    return (void*) sp;
}

// static void thread_switch() {
//     uthread old_id = curthread_id;
    
//     assert(queue_size(runqueue) > 0);

//     curthread_id = queue_dequeue(runqueue);
//     queue_enqueue(runqueue, old_id);

//     context_switch(&threads[old_id]->sp, threads[curthread_id]->sp);
// }

static void thread_sleep() {
    uthread old_id = curthread_id;
    assert(queue_size(runqueue) > 0);
    curthread_id = queue_dequeue(runqueue);
    context_switch(&threads[old_id]->sp, threads[curthread_id]->sp);
}

static void thread_wake(uthread thread) {
    int result = queue_enqueue(runqueue, thread);
    if (result)
        return; // Queue is full. Should not get here.
}

static void thread_destroy(uthread thread) {
    assert(thread != 0); // should not be destroying main thread
    assert(thread != curthread_id); // should not be destroying current thread

    free(threads[thread]->stack_end);
    free(threads[thread]);
    threads[thread] = NULL;
    thread_count--;
}

void uthread_init(sched_policy policy, size_t stack_sz) {
    if (initialized)
        return;
    initialized = true;

    struct thread *main_thread = malloc(sizeof(struct thread));
    main_thread->stack_end = NULL;
    main_thread->sp = NULL;

    threads[0] = main_thread;
    thread_count++;

    switch(policy) {
        case FIFO:
            runqueue = queue_create(MAX_THREADS);
            curthread_id = 0; // main thread is currently running
            
        default:
            // not yet implemented
    }

    stack_size = stack_sz;
}

int uthread_create(uthread *thread, void* (*func)(void*), void *args) {
    if (!initialized)
        uthread_init(FIFO, DEFAULT_STACK_SIZE);

    if (thread == NULL)
        return -1; // invalid uthread pointer  

    if (thread_count + 1 > MAX_THREADS)
        return -1; // too many threads
        
    // allocate and setup thread struct
    void* stack = malloc(stack_size);
    if (stack == NULL)
        return -1; // out of memory

    struct thread *t = malloc(sizeof(struct thread));
    if (t == NULL) {
        free(stack);
        return -1; // out of memory
    }

    t->func = func;
    t->args = args;
    t->stack_end = stack; 
    void *stack_bottom = (void*) ((uintptr_t) t->stack_end + stack_size);
    t->sp = thread_setup_stack(stack_bottom);

    // add to threads array
    while (threads[last_id] != NULL) {
        last_id++;
        if (last_id == MAX_THREADS)
            last_id = 1;
    }
    threads[last_id] = t;
    thread_count++;
    *thread = last_id;

    // add thread to runqeue
    thread_wake(last_id);

    return 0;
}

int uthread_join(uthread thread, void **retval) {
    struct thread *joining_thread = threads[thread];
    if (joining_thread == NULL)
        return -1; // invalid id

    joining_thread->join_id = curthread_id; 
    
    // block if joining thread has not terminated yet
    if (!joining_thread->terminated) {
        thread_sleep();
    }
    assert(joining_thread->terminated);

    // give return value of joining thread if not NULL
    if (retval != NULL)
        *retval = joining_thread->retval;

    // cleanup joining thread
    thread_destroy(thread);

    return 0;
}

void uthread_exit(void *retval) {
    struct thread *t = threads[curthread_id];

    thread_wake(t->join_id);

    t->retval = retval;
    t->terminated = true;

    thread_sleep();
}