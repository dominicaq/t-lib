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

void thread2(void *arg) {
    (void)arg;

    forever_loop = 0;
    printf("thread2 ended loop\n");
    uthread_yield();
}

void thread1(void *arg) {
    (void)arg;

    printf("thread1 entering loop\n");
    uthread_create(thread2, NULL);
    // Stuck here until preempt interrupts loop
    while(forever_loop == 1) { printf("stuck\n"); }
    printf("thread1 exited\n");
    uthread_yield();
}

int main(void) {
    uthread_run(true, thread1, NULL);
    return 0;
}