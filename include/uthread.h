 /**
 * @file uthread.h
 * @brief Lightweight user-level thread library for Linux/x86
 * 
 * This library provides lightweight, cooperatively-scheduled user-level
 * threads (uthreads) that run within a single OS process and thread.
 * 
 * Threads must explicitly give up the CPU via uthread_yield(), uthread_join(),
 * or uthread_exit(). There is no preemption, so a running thread cannot be
 * interrupted by the scheduler.
 * 
 * The following scheduling policies are supported:
 * 
 *  - First-In, First-Out (FIFO): Threads run in creation order. A yielding
 *     thread is added at the end of the runqueue.
 *  - Priority Scheduling (PS): Threads run in order of priority (higher first).
 *     Ties are resolved non-deterministically. Note: A running thread is not
 *     preempted if a higher-priority thread becomes ready.
 * 
 * Because scheduling is cooperative and all threads share a single CPU core,
 * this library does not provide synchronization primitives (mutexes,
 * semaphores, etc.). Users must ensure threads do not yield while inside a
 * critical section to prevent race conditions.
 * 
 * @author Raimi Misalucha
 */

#ifndef UTHREAD_H 
#define UTHREAD_H    

#include "thread.h"
#include <stdlib.h>
#include <stdbool.h>

// Maximum number of concurrent threads
#define MAX_THREADS 64

// Default stack size per thread (64KB)
#define DEFAULT_STACK_SIZE 65536

// Default scheduling policy if uthread_init() is not called explicitly
#define DEFAULT_SCHEDULING_POLICY FIFO

// Maximum and minimum priorities of threads
#define MAX_PRIORITY 20
#define MIN_PRIORITY -20

typedef enum {
    FIFO, // First-In-First-Out
    PS    // Priority Scheduling 
} sched_policy;

/**
 * @brief Initializes the uthread library with specified parameters.
 * 
 * This function must be called before creating any threads to set a non-default
 * scheduling policy or stack size. If not called explicitly, the first call to
 * uthread_create() will initialize with defaults.
 * 
 * Only the first call to this function sets paramaters, so it should not be
 * called multiple times.
 * 
 * @param policy Scheduling policy to use (FIFO or PS).
 * @param stack_sz Stack size in bytes for each thread.
 * 
 * @warning Once initialized, the scheduling policy and stack size are fixed.
 */
void uthread_init(sched_policy policy, size_t stack_sz);

/**
 * @brief Creates a new thread.
 * 
 * Creates a new thread that will execute the given function with the provided
 * arguments. The new thread is immediately added to the runqueue and will
 * run according to the current scheduling policy.
 * 
 * @param[out] thread Pointer to store the new thread's ID. Cannot be NULL.
 * @param[in] func Function to execute in the new thread.
 * @param[in] args Argument to pass to func. Can be NULL.
 * @param[in] priority Priority of new thread. Only has effect for priority
 *                     scheduling. Must be in range [MIN_PRIORITY, MAX_PRIORITY].
 * 
 * @return 0 on success, -1 on error
 * 
 * @retval 0 Success: thread created and scheduled
 * @retval -1 Error occurred:
 *            - thread pointer is NULL
 *            - Maximum thread limit (MAX_THREADS) reached
 *            - Invalid priority
 *            - Memory allocation failed
 * 
 * @note If uthread_init() was not called, this function initializes with defaults.
 */
int uthread_create(uthread *thread, void* (*func)(void*), void *args, int priority);

/**
 * @brief Waits for a thread to terminate and collect its return value.
 * 
 * Blocks the calling thread until the specified thread terminates. The joined
 * thread's resources are released after this call. A thread can only be joined
 * once.
 * 
 * @param[in] utid ID of the thread to join
 * @param[out] retval Pointer to store the joined thread's return value.
 *                    May be NULL if return value is not needed.
 * 
 * @return 0 on success, -1 on error
 * 
 * @retval 0 Success: thread joined and cleaned up
 * @retval -1 Error occurred:
 *            - Invalid thread ID
 *            - Thread does not exist
 *            - Thread is detached
 *            - Thread is already being joined by another thread
 * 
 * @warning Joining a thread that is already joined or detached is an error.
 * 
 * @note The calling thread cannot run until the joined thread terminates.
 */
int uthread_join(uthread utid, void **retval);

/**
 * @brief Terminates the calling thread with a return value.
 * 
 * Causes the calling thread to exit immediately. The return value can be
 * collected by another thread via uthread_join(). This function is implicitly
 * called when a thread function returns normally.
 * 
 * @param[in] retval Return value to provide to joining thread. Can be NULL.
 * 
 * @note This function does not return to the caller.
 * @note If called from the main thread, the entire program is terminated. 
 * @note If the thread is being joined, the joining thread is awakened.
 * @note If the thread is detached, resources are automatically released.
 */
void uthread_exit(void *retval);

/**
 * @brief Detaches a thread so its resources are automatically released upon
 * termination.
 * 
 * Marks the specified thread as detached. When a detached thread terminates,
 * its resources are automatically released. A detached thread cannot be joined.
 * 
 * @param[in] utid ID of the thread to detach
 * 
 * @return 0 on success, -1 on error
 * 
 * @retval 0 Success: thread detached
 * @retval -1 Error occurred:
 *            - Invalid thread ID
 *            - Thread does not exist
 *            - Thread is already detached
 *            - Thread is already being joined
 * 
 * @warning Once a thread is detached, its return value cannot be retrieved.
 */
int uthread_detach(uthread utid);

/**
 * @brief Voluntarily yields the CPU to the next scheduled thread.
 * 
 * Causes the calling thread to give up the CPU and be placed back in the
 * runqueue according to the scheduling policy. The next thread in the runqueue
 * will be selected to run.
 * 
 * @note This is the primary mechanism for cooperative scheduling. Threads
 *       should call this periodically to allow other threads to run.
 * 
 * @note If the runqueue is empty, execution stays with the calling thread.
 * 
 * @warning Running threads that don't yield will starve other threads.
 */
void uthread_yield();

#endif