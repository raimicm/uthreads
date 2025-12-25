#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uthread.h>

void* print_int(void *args) {
    int n = *((int*) args);
    for (int i = 0; i < 5; i++) {
        printf("%d\n", n);
        usleep(100000);
    }
    return NULL;
}

int main() {
    uthread thread1, thread2;

    void *arg1 = malloc(sizeof(int));
    *((int*) arg1) = 1;

    void *arg2 = malloc(sizeof(int));
    *((int*) arg2) = 2;

    int err = uthread_create(&thread1, print_int, arg1);
    if (err) {
        printf("Error creating thread1.\n");
        return 1;
    }
    printf("Created thread1.\n");

    err = uthread_create(&thread2, print_int, arg2);
    if (err) {
        printf("Error creating thread2.\n");
        return 1;
    }
    printf("Created thread2.\n");

    err = uthread_join(thread1, NULL);
    if (err) {
        printf("Error joining thread1.\n");
        return 1;
    }
    printf("Joined thread1.\n");

    uthread_join(thread2, NULL);
    if (err) {
        printf("Error joining thread2.\n");
        return 1;
    }
    printf("Joined thread2.\n");

    free(arg1);
    free(arg2);

    return 0;
}