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

volatile int forever_loops = 1;

void thread4(void *arg) {
    (void)arg;

    forever_loops = 0;
    printf("thread4, exiting loops\n");
    // exit(0)
}

void thread3(void *arg) {
    (void)arg;

    uthread_create(thread4, NULL);
    printf("thread3 loop\n");
    while(forever_loops == 1) { }
}

void thread2(void *arg) {
    (void)arg;

    uthread_create(thread3, NULL);
    printf("thread2 loop\n");
    while(forever_loops == 1) { }
}

void thread1(void *arg) {
    (void)arg;

    printf("thread1 entering loops\n");
    uthread_create(thread2, NULL);
    // Stuck here until preempt interrupts loop
    while(forever_loops == 1) { }
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