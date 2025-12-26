#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uthread.h>

void* print_hello(void *args) {
    (void) args;
    for (int i = 0; i < 10; i++) {
        printf("hello");
        usleep(50000);
        uthread_yield();
    }
    return NULL;
}

void* print_world(void *args) {
    (void) args;
    for (int i = 0; i < 10; i++) {
        printf(" world\n");
        usleep(50000);
        uthread_yield();
    }
    return NULL;
}

int main() {
    uthread thread1, thread2;

    uthread_init(PS, DEFAULT_STACK_SIZE);

    int err = uthread_create(&thread1, print_hello, NULL, 2);
    if (err) {
        printf("Error creating thread1.\n");
        return 1;
    }

    err = uthread_create(&thread2, print_world, NULL, 1);
    if (err) {
        printf("Error creating thread2.\n");
        return 1;
    }

    uthread_yield();

    return 0;
}