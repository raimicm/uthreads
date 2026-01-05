# uthreads

This library provides lightweight, cooperatively-scheduled user-level threads (`uthreads`) that run within a single OS process and thread.


## Requirements

To build and use this library, you will need:

- x86 processor
- Linux
- Make
- GCC
- Git

## Build Instructions

### Running examples:

#### Step 1: Clone the Repository

Clone this repository:

```bash
git clone https://github.com/raimicm/uthreads.git
```

#### Step 2: Build

Change directory into the root folder of the cloned repository and run make to compile the examples:

```bash
cd uthreads
make all
```

#### Step 3: Run

Now you can run an example. If you want to run `join_example`, you would use the following commands.
```bash
cd build
./join_example
```
## Documentation

### Available Functions:

These are all the core API functions:

| Function | Brief Description |
| :--- | :--- |
| `int uthread_init(sched_policy policy, size_t stack_sz)` | Initializes the uthread library with specified parameters. |
| `int uthread_create(uthread *thread, void* (*func)(void*), void *args, int priority)` | Creates a new thread. |
| `int uthread_join(uthread utid, void **retval)` | Waits for a thread to terminate and collect its return value. |
| `void uthread_exit(void *retval)` | Terminates the calling thread with a return value. |
| `void uthread_detach(uthread utid)` | Detaches a thread so its resources are automatically released upon termination. |
| `void uthread_yield()` | Voluntarily yields the CPU to the next scheduled thread. |

See `include/uthreads.h` for the full API documentation. 

### Cooperative Scheduling:

Threads must explicitly give up the CPU via `uthread_yield()`, `uthread_join()`, or `uthread_exit()`. There is no preemption, so a running thread cannot be interrupted by the scheduler.

### Supported Scheduling Policies:

 - **First-In, First-Out (`FIFO`)**: Threads run in creation order. A yielding thread is added at the end of the runqueue.
 - **Priority Scheduling (`PS`)**: Threads run in order of priority (higher first). Ties are resolved non-deterministically. *Note: A running thread is not preempted if a higher-priority thread becomes ready.* 

### Concurrency and Safety:

Because scheduling is cooperative and all threads share a single CPU core, this library does not provide synchronization primitives (mutexes, semaphores, etc.). Users must ensure threads do not yield while inside a critical section to prevent race conditions.