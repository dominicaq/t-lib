/*
 * Preemption simple test
 *
 * Test preempted interupts by having one thread hog resources until the other
 * thread breaks its loop
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int forever_loop = 1;

void thread4(void *arg) {
    (void)arg;

    forever_loop = 0;
    printf("thread4, exiting loop\n");
    // exit(0)
}

void thread3(void *arg) {
    (void)arg;

    uthread_create(thread4, NULL);
    printf("thread3\n");
}

void thread2(void *arg) {
    (void)arg;

    uthread_create(thread3, NULL);
    printf("thread2\n");
}

void thread1(void *arg) {
    (void)arg;

    printf("thread1 entering loop\n");
    uthread_create(thread2, NULL);
    // Stuck here until preempt interrupts loop
    while(forever_loop == 1) { }
}

int main(int argc, char **argv) {
    bool use_preempt = false;

    // Turn off preempt if user specifes
    if (argc > 1) {
        if (atoi(argv[1]) > 0) {
            use_preempt = true;
        }
    }

    uthread_run(use_preempt, thread1, NULL);
    return 0;
}