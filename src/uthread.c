#include "uthread.h"
#include "context_switch.h"
#include "queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define UTHREAD_DETACHED -2

typedef enum {
    RDY, // Ready
    RUN, // Running
    SLP, // Sleeping
    ZMB  // Zombie 
} thread_state;

struct thread {
    void* stack_end;
    void* sp;
    void* (*func)(void*);
    void* args;
    void* retval;
    thread_state state;
    int priority;
    uthread join_id;
};

bool initialized = false;
sched_policy scheduling_policy = DEFAULT_SCHEDULING_POLICY;
size_t stack_size = DEFAULT_STACK_SIZE;

struct thread *threads[MAX_THREADS + 1];
unsigned last_id = 0;
unsigned thread_count = 0;

uthread cur_utid;
queue fifo_runqueue;
priority_queue ps_runqueue;
queue zombies;

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

static void thread_switch(thread_state state) {
    assert(state != RUN);
    assert(threads[cur_utid]->state == RUN);
    uthread old_id = cur_utid;

    switch (scheduling_policy) {
        case FIFO:
            cur_utid = queue_dequeue(fifo_runqueue);
            assert(cur_utid >= 0);
            threads[cur_utid]->state = RUN;

            if (state == RDY)
                queue_enqueue(fifo_runqueue, old_id);
            threads[old_id]->state = state;
            break;
        case PS:
            cur_utid = priority_queue_dequeue(ps_runqueue);
            assert(cur_utid >= 0);
            threads[cur_utid]->state = RUN;

            if (state == RDY)
                priority_queue_enqueue(ps_runqueue, old_id, threads[old_id]->priority);
            threads[old_id]->state = state;
            break;
        default:
            // not yet implemented
    }

    context_switch(&threads[old_id]->sp, threads[cur_utid]->sp);
}

static void thread_wake(uthread utid) {
    struct thread *t = threads[utid];
    assert(t->state == SLP);
    t->state = RDY;
    switch (scheduling_policy) {
        case FIFO:
            assert(!queue_enqueue(fifo_runqueue, utid));
            break;
        case PS:
            assert(!priority_queue_enqueue(ps_runqueue, utid, t->priority));
            break;
        default:
            // not yet implemented

    }
}

static struct thread *thread_create(void* (*func)(void*), void* args, int priority) {
    struct thread *t = malloc(sizeof(struct thread));
    if (t == NULL) 
        return NULL; // out of memory

    // allocate and setup thread struct
    t->stack_end = malloc(stack_size);
    if (t->stack_end == NULL) {
        free(t);
        return NULL; // out of memory
    }

    t->func = func;
    t->args = args;
    t->retval = NULL;
    t->state = SLP;
    t->priority = priority;
    t->join_id = -1;

    void *stack_bottom = (void*) ((uintptr_t) t->stack_end + stack_size);
    t->sp = thread_setup_stack(stack_bottom);

    return t;
}

static void thread_destroy(uthread utid) {
    assert(utid != 0); // should not be destroying main thread
    assert(utid != cur_utid); // should not be destroying current thread

    struct thread *t = threads[utid];
    assert(t->state == ZMB);

    free(t->stack_end);
    free(t);

    threads[utid] = NULL;
    thread_count--;
}

void* thread_reaper(void *args) {
    (void) args;
    for (;;) {
        int zombie_count = queue_size(zombies);
        for (int i = 0; i < zombie_count; i++) {
            uthread utid = queue_dequeue(zombies);
            thread_destroy(utid);
        }
        thread_switch(RDY);
    }
    return NULL;
}

void uthread_init(sched_policy policy, size_t stack_sz) {
    if (initialized)
        return;
    initialized = true;

    stack_size = stack_sz;
    scheduling_policy = policy;

    switch(policy) {
        case FIFO:
            fifo_runqueue = queue_create(MAX_THREADS);
            break;
        case PS:
            ps_runqueue = priority_queue_create(MAX_THREADS);
            break;
        default:
            // not yet implemented
    }

    zombies = queue_create(MAX_THREADS);

    struct thread *main_thread = malloc(sizeof(struct thread));
    main_thread->stack_end = NULL;
    main_thread->sp = NULL;
    main_thread->state = RUN;
    main_thread->join_id = -1;

    cur_utid = 0; // main thread is currently running
    threads[0] = main_thread;
    thread_count++;

    struct thread *reaper_thread = thread_create(thread_reaper, NULL, MIN_PRIORITY);
    threads[MAX_THREADS] = reaper_thread;
    thread_count++;

    thread_wake(MAX_THREADS); // add reaper thread to runqueue
}

int uthread_create(uthread *thread, void* (*func)(void*), void *args, int priority) {
    if (!initialized)
        uthread_init(FIFO, DEFAULT_STACK_SIZE);

    if (thread == NULL)
        return -1; // invalid uthread pointer  

    if (thread_count + 1 > MAX_THREADS)
        return -1; // too many threads

    if (priority > MAX_PRIORITY || priority < MIN_PRIORITY)
        return -1; // invalid priority

    struct thread *t = thread_create(func, args, priority);
    if (t == NULL)
        return -1; // out of memory

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
    if (t->state != ZMB) {
        thread_switch(SLP);
    }
    assert(t->state == ZMB);

    // give return value of joining thread if not NULL
    if (retval != NULL)
        *retval = t->retval;

    // cleanup joining thread
    thread_destroy(utid);

    return 0;
}

void uthread_exit(void *retval) {
    if (cur_utid == 0)
        exit(0); // terminate process if main thread calls uthread_exit

    struct thread *t = threads[cur_utid];

    if (t->join_id == UTHREAD_DETACHED) {
        threads[cur_utid] = NULL;
        queue_enqueue(zombies, cur_utid);
    } else if (t->join_id == -1) {
        t->retval = retval;
    } else {
        thread_wake(t->join_id);
        t->retval = retval;
    }
    thread_switch(ZMB);
}

int uthread_detach(uthread utid) {
    if (utid < 0 || utid >= MAX_THREADS)
        return -1; // invalid id

    struct thread *t = threads[utid];
    if (t == NULL)
        return -1; // invalid id

    if (t->join_id != -1)
        return -1; // thread is already detached or marked to join

    t->join_id = UTHREAD_DETACHED; // mark thread as detached

    return 0;
}

void uthread_yield() {
    thread_switch(RDY);
}