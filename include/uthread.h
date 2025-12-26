#ifndef UTHREAD_H 
#define UTHREAD_H    

/*
 * User-level thread library.
 *
 * The following scheduling policies are available:
 *   1. FIFO
 *   2. Priority
 *   3. Round Robin
 *   4. Complemtely Fair Scheduler
 *   5. Multi-Level Feedback Queue
 */

#include <stdlib.h>
#include <stdbool.h>

#define MAX_THREADS 64
#define DEFAULT_STACK_SIZE 65536
#define DEFAULT_SCHEDULING_POLICY FIFO
#define MAX_PRIORITY 20
#define MIN_PRIORITY -20

typedef int uthread;

typedef enum {
    FIFO, // First-In-First-Out
    PS,    // Priority Scheduling 
    RR,   // Round Robin
    CFS,   // Completely Fair Scheduler
    MLFQ  // Multi-Level Feedback Queue
} sched_policy;

/* 
 * Inializes uthreads and sets uthread settings. The user should call this
 * function before creating a uthread to set the scheduling policy and stack
 * size for uthreads. Otherwise, the scheduling policy and stack size will be 
 * set to default (DEFAULT_STACK_SIZE and DEFAULT_SCHEDULIING_POLICY).
 * 
 * Parameters:
 *     - policy: thread scheduling policy
 *     - stack_siz: thread stack size 
 */
void uthread_init(sched_policy policy, size_t stack_sz);

/*
 * Creates a uthread to run a user-specified function.
 * 
 * Parameters:
 *     - thread: pointer to a location to store the utid (uthread id) of the
 *               created uthread
 *     - func: function for uthread to thread
 *     - args: pointer to arguments for func
 *     - priority: priority level for this uthread. Only necessary for priority
 *                 scheduling (PS) and completely fair scheduling (CFS).
 * 
 * Returns:
 *     - 0 if successful, -1 if error occured
 */
int uthread_create(uthread *thread, void* (*func)(void*), void *args, int priority);

/*
 * Joins a uthread to calling uthread. This collects its return value and releases 
 * its memory. The calling uthread blocks until the other uthread has exited.
 * 
 * Parameters:
 *     - utid: id of uthread to join
 *     - retval: pointer to a location to store the return value of the joined
 *               uthread. May be NULL if the return value is not required.
 */
int uthread_join(uthread utid, void **retval);

/*
 * Causes the calling uthread to exit with some return value that can be
 * collected by another uthread through uthread_join. This function is
 * implicitly called when the current uthread completes executing.
 * 
 * Parameters:
 *     - retval: pointer to the calling uthread's return value
 */
void uthread_exit(void *retval);

/*
 * Detaches the calling uthread so its resources are automatically released
 * upon termination.
 * 
 * Parameters:
 *     - utid: id of uthread to detach
 * 
 * Returns:
 *     - 0 if successful, -1 if error occured
 */
int uthread_detach(uthread utid);

/*
 * Blocks the current uthread, yielding the CPU to the next scheduled uthread.
 */
void uthread_yield();

#endif