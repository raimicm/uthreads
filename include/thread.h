#ifndef THREAD_H 
#define THREAD_H    

typedef int uthread;

typedef enum {
    RDY, // Ready
    RUN, // Running
    SLP, // Sleeping
    ZMB  // Zombie 
} thread_state;

struct thread {
    uthread id;
    void* stack_end;
    void* sp;
    void* (*func)(void*);
    void* args;
    void* retval;
    thread_state state;
    int priority;
    uthread join_id;
};

#endif