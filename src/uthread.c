#include "uthread.h"
#include "context_switch.h"
#include "queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define UTHREAD_DETACHED -2

bool initialized = false;
sched_policy scheduling_policy = FIFO;
size_t stack_size = DEFAULT_STACK_SIZE;

struct thread *threads[MAX_THREADS];
unsigned last_id = 0;
unsigned thread_count = 0;

uthread cur_utid;
queue runqueue;

void thread_execute() {
    struct thread *curthread = threads[cur_utid];

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

// static void thread_yield() {
//     uthread old_id = cur_utid;
    
//     assert(queue_size(runqueue) > 0);

//     cur_utid = queue_dequeue(runqueue);
//     queue_enqueue(runqueue, old_id);

//     context_switch(&threads[old_id]->sp, threads[cur_utid]->sp);
// }

static void thread_sleep() {
    uthread old_id = cur_utid;
    assert(!threads[old_id]->sleeping);
    threads[old_id]->sleeping = true;

    cur_utid = queue_dequeue(runqueue);
    assert(cur_utid >= 0);
    threads[cur_utid]->sleeping = false;

    context_switch(&threads[old_id]->sp, threads[cur_utid]->sp);
}

static void thread_wake(uthread utid) {
    assert(threads[utid]->sleeping);
    threads[utid]->sleeping = false;
    assert(!queue_enqueue(runqueue, utid));
}

static void thread_destroy(uthread utid) {
    assert(utid != 0); // should not be destroying main thread
    assert(utid != cur_utid); // should not be destroying current thread

    free(threads[utid]->stack_end);
    free(threads[utid]);
    threads[utid] = NULL;
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
            cur_utid = 0; // main thread is currently running
            
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

    t->stack_end = stack; 
    t->func = func;
    t->args = args;
    t->retval = NULL;
    t->terminated = false;
    t->join_id = -1;
    t->sleeping = true;

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

int uthread_join(uthread utid, void **retval) {
    if (utid < 0 || utid >= MAX_THREADS)
        return -1; // invalid id

    struct thread *t = threads[utid];
    if (t == NULL)
        return -1; // invalid id

    if (t->join_id == UTHREAD_DETACHED || t->join_id >= 0)
        return -1; // thread is detached or already marked to join

    t->join_id = cur_utid; 
    
    // block if joining thread has not terminated yet
    if (!t->terminated) {
        thread_sleep();
    }
    assert(t->terminated);
    assert(t->sleeping);

    // give return value of joining thread if not NULL
    if (retval != NULL)
        *retval = t->retval;

    // cleanup joining thread
    thread_destroy(utid);

    return 0;
}

void uthread_exit(void *retval) {
    struct thread *t = threads[cur_utid];

    if (t->join_id == UTHREAD_DETACHED) {
        threads[cur_utid] = NULL;
    } else if (t->join_id == -1) {
        t->retval = retval;
        t->terminated = true;
        thread_sleep();
    } else {
        thread_wake(t->join_id);
        t->retval = retval;
        t->terminated = true;
        thread_sleep();
    }
}
