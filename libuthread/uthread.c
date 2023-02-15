#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

// Thread struct
// =============================================================================
typedef struct uthread_tcb {
    uthread_ctx_t ctx; // Thread context
    void  *stack_head; // Stack
} uthread_tcb;

// Scheduler
// =============================================================================
queue_t     ready_queue;
queue_t     blocked_queue;
queue_t     zombie_queue;
uthread_tcb *current_thread = NULL;

struct uthread_tcb *uthread_current(void) {
    return current_thread;
}

void uthread_swap_threads(void) {
    if (queue_length(ready_queue) == 0) {
        return;
    }

    uthread_tcb *prev_thread = current_thread;
    queue_dequeue(ready_queue, (void**)&current_thread);
    uthread_ctx_switch(&(prev_thread->ctx), &(current_thread->ctx));
}

void uthread_yield(void) {
    queue_enqueue(ready_queue, current_thread);
    uthread_swap_threads();
}

void uthread_exit(void) {
    queue_enqueue(zombie_queue, current_thread);
    uthread_swap_threads();
}

int uthread_create(uthread_func_t func, void *arg) {
    uthread_tcb *new_thread = malloc(sizeof(uthread_tcb));
    if (new_thread == NULL) {
        // ERROR: Bad malloc
        return -1;
    }

    // Initialize new thread
    new_thread->stack_head = uthread_ctx_alloc_stack();
    if (new_thread->stack_head == NULL) {
        // ERROR: Failed to alloc stack
        return -1;
    }

    int retval = uthread_ctx_init(&(new_thread->ctx), new_thread->stack_head, func, arg);
    if (retval <= -1) {
        // ERROR: Init context failed
        return -1;
    }

    queue_enqueue(ready_queue, new_thread);
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    // Init scheduler
    ready_queue   = queue_create();
    blocked_queue = queue_create();
    zombie_queue  = queue_create();

    // Create user and idle thread
    int idle_retval = uthread_create(NULL, NULL);
    int user_retval = uthread_create(func, arg);
    if (idle_retval < 0 || user_retval < 0) {
        // ERROR: Thread creation failed
        return -1;
    }

    // Set idle thread as initial current thread
    queue_dequeue(ready_queue, (void**)&current_thread);

    // Idle loop
    while (1) {
        // Exit the idle loop when ready queue is empty
        int num_threads = queue_length(ready_queue);
        if (num_threads == 0) {
            break;
        }
        
        // Swap to the next thread
        preempt_start(preempt);
        uthread_yield();
        preempt_stop();
    }

    // Free zombies
    while (queue_length(zombie_queue) != 0) {
        uthread_tcb* zombie_thread;
        queue_dequeue(zombie_queue, (void**)&zombie_thread);
        uthread_ctx_destroy_stack(zombie_thread->stack_head);
        free(zombie_thread);
    }

    // Free current thread
    uthread_ctx_destroy_stack(current_thread->stack_head);
    free(current_thread);

    // Destroy queues
    queue_destroy(zombie_queue);
    queue_destroy(blocked_queue);
    queue_destroy(ready_queue);
    return 0;
}

void uthread_block(void) {
    // Block the current thread and swap tp idle ctx
    queue_enqueue(blocked_queue, current_thread);
    uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread) {
    // Delete from blocked queue and add to ready queue if it existed
    int retval = queue_delete(blocked_queue, uthread);
    if (retval > 0) {
        queue_enqueue(ready_queue, uthread);
    }
}