#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uthread.h>

void* print_hello(void *args) {
    (void) args;
    for (int i = 0; i < 10; i++) {
        printf("hello\n");
        usleep(100000);
    }
    return NULL;
}

void* print_world(void *args) {
    (void) args;
    for (int i = 0; i < 10; i++) {
        printf("world\n");
        usleep(100000);
    }
    return NULL;
}

int main() {
    uthread thread1, thread2;

    int err = uthread_create(&thread1, print_hello, NULL);
    if (err) {
        printf("Error creating thread1.\n");
        return 1;
    }
    printf("Created thread1.\n");

    err = uthread_create(&thread2, print_world, NULL);
    if (err) {
        printf("Error creating thread2.\n");
        return 1;
    }
    printf("Created thread2.\n");

    uthread_yield();

    return 0;
}