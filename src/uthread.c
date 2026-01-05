#include "uthread.h"
#include "context_switch.h"
#include "thread_queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define UTHREAD_DETACHED -2

bool initialized = false;
sched_policy scheduling_policy = DEFAULT_SCHEDULING_POLICY;
size_t stack_size = DEFAULT_STACK_SIZE;

struct thread *threads[MAX_THREADS];
unsigned last_id = 0;
unsigned thread_count = 0;

thread_queue fifo_runqueue;
thread_pqueue ps_runqueue;
thread_queue zombies;

struct thread *curthread;
struct thread *reaper_thread;

void thread_execute() {
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
    assert(curthread->state == RUN);
    struct thread *oldthread = curthread;
    struct thread *newthread = NULL;
    
    switch (scheduling_policy) {
        case FIFO:
            newthread = thread_queue_dequeue(fifo_runqueue);
            if (newthread == NULL)
                return; // runqueue is empty
            newthread->state = RUN;
            curthread = newthread;

            if (state == RDY)
               thread_queue_enqueue(fifo_runqueue, oldthread);
            oldthread->state = state;
            break;
        case PS:
            newthread = thread_pqueue_dequeue(ps_runqueue);
            if (newthread == NULL)
                return; // runqueue is empty
            newthread->state = RUN;
            curthread = newthread;

            if (state == RDY)
               thread_pqueue_enqueue(ps_runqueue, oldthread);
            oldthread->state = state;
            break;
        default:
            // not yet implemented
    }

    context_switch(&oldthread->sp, curthread->sp);
}

static void thread_wake(struct thread* t) {
    assert(t->state == SLP);
    t->state = RDY;
    switch (scheduling_policy) {
        case FIFO:
            assert(!thread_queue_enqueue(fifo_runqueue, t));
            break;
        case PS:
            assert(!thread_pqueue_enqueue(ps_runqueue, t));
            break;
        default:
            // not yet implemented

    }
}

static struct thread *thread_create(uthread id, void* (*func)(void*), void* args, int priority) {
    struct thread *t = malloc(sizeof(struct thread));
    if (t == NULL) 
        return NULL; // out of memory

    // allocate and setup thread struct
    t->stack_end = malloc(stack_size);
    if (t->stack_end == NULL) {
        free(t);
        return NULL; // out of memory
    }

    t->id = id;
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

static void thread_destroy(struct thread *t) {
    assert(t->id != 0); // should not be destroying main thread
    assert(t != curthread); // should not be destroying current thread
    assert(t->state == ZMB);

    threads[t->id] = NULL;

    free(t->stack_end);
    free(t);

    thread_count--;
}

void* thread_reaper(void *args) {
    (void) args;
    for (;;) {
        int zombie_count = thread_queue_size(zombies);
        for (int i = 0; i < zombie_count; i++) {
            struct thread* t = thread_queue_dequeue(zombies);
            thread_destroy(t);
        }
        thread_switch(SLP);
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
            fifo_runqueue = thread_queue_create(MAX_THREADS);
            break;
        case PS:
            ps_runqueue = thread_pqueue_create(MAX_THREADS);
            break;
        default:
            // not yet implemented
    }

    zombies = thread_queue_create(MAX_THREADS);

    struct thread *main_thread = malloc(sizeof(struct thread));
    main_thread->id = 0;
    main_thread->stack_end = NULL;
    main_thread->sp = NULL;
    main_thread->state = RUN;
    main_thread->join_id = -1;

    curthread = main_thread; // main thread is currently running
    threads[0] = main_thread;
    thread_count++;

    reaper_thread = thread_create(MAX_THREADS, thread_reaper, NULL, MAX_PRIORITY);
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

    // add to threads array
    while (threads[last_id] != NULL) {
        last_id++;
        if (last_id == MAX_THREADS)
            last_id = 1;
    }

    struct thread *t = thread_create(last_id, func, args, priority);
    if (t == NULL)
        return -1; // out of memory

    threads[last_id] = t;
    thread_count++;
    *thread = last_id;

    // add thread to runqeue
    thread_wake(t);

    return 0;
}

int uthread_join(uthread utid, void **retval) {
    if (utid < 0 || utid >= MAX_THREADS)
        return -1; // invalid id

    struct thread *t = threads[utid];
    if (t == NULL)
        return -1; // thread does not exist

    if (t->join_id == UTHREAD_DETACHED || t->join_id >= 0)
        return -1; // thread is detached or already marked to join

    t->join_id = curthread->id; 
    
    // block if joining thread has not terminated yet
    if (t->state != ZMB) {
        thread_switch(SLP);
    }
    assert(t->state == ZMB);

    // give return value of joining thread if not NULL
    if (retval != NULL)
        *retval = t->retval;

    // cleanup joining thread
    thread_destroy(t);

    return 0;
}

void uthread_exit(void *retval) {
    if (curthread->id == 0)
        exit(0); // terminate process if main thread calls uthread_exit

    if (curthread->join_id == UTHREAD_DETACHED) {
        thread_queue_enqueue(zombies, curthread);
        if (reaper_thread->state == SLP)
            thread_wake(reaper_thread); // add reaper to runqueue to clean up zombie
    } else if (curthread->join_id == -1) {
        curthread->retval = retval;
    } else {
        thread_wake(threads[curthread->join_id]);
        curthread->retval = retval;
    }
    thread_switch(ZMB);
}

int uthread_detach(uthread utid) {
    if (utid < 0 || utid >= MAX_THREADS)
        return -1; // invalid id

    struct thread *t = threads[utid];
    if (t == NULL)
        return -1; // thread does not exist

    if (t->join_id != -1)
        return -1; // thread is already detached or marked to join

    t->join_id = UTHREAD_DETACHED; // mark thread as detached

    return 0;
}

void uthread_yield() {
    thread_switch(RDY);
}