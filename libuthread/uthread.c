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
    int is_zombie;
    uthread_ctx_t ctx; // Thread context
    void  *stack_head; // Stack
} uthread_tcb;

// Scheduler
// =============================================================================
queue_t     ready_queue;
queue_t     blocked_queue;
uthread_tcb *current_thread = NULL;
static uthread_ctx_t idle_ctx;

struct uthread_tcb *uthread_current(void) {
    return current_thread;
}

void uthread_swap_to_idle(void) {
    uthread_ctx_switch(&(current_thread->ctx), &(idle_ctx));
}

void uthread_yield(void) {
    queue_enqueue(ready_queue, current_thread);
    uthread_swap_to_idle();
}

void uthread_exit(void) {
    current_thread->is_zombie = 1;
    queue_enqueue(ready_queue, current_thread);
    uthread_swap_to_idle();
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

    new_thread->is_zombie = 0;
    queue_enqueue(ready_queue, new_thread);
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    // Init scheduler
    ready_queue   = queue_create();
    blocked_queue = queue_create();

    // Create initial the user created thread
    int retval = uthread_create(func, arg);
    if (retval <= -1) {
        // ERROR: Thread creation failed
        return -1;
    }

    // Idle loop
    while (1) {
        // Exit the idle loop when ready queue is empty
        int num_threads = queue_length(ready_queue);
        if (num_threads == 0) {
            break;
        }
        
        preempt_stop();
        queue_dequeue(ready_queue, (void**)&current_thread);

        // Free current thread if its a zombie
        if (current_thread->is_zombie > 0) {
            uthread_ctx_destroy_stack(current_thread->stack_head);
            free(current_thread);
            continue;
        }

        // Swap to the next context
        preempt_start(preempt);
        uthread_ctx_switch(&(idle_ctx), &(current_thread->ctx));
        preempt_stop();
    }

    queue_destroy(blocked_queue);
    queue_destroy(ready_queue);
    return 0;
}

void uthread_block(void) {
    // Block the current thread and swap tp idle ctx
    queue_enqueue(blocked_queue, current_thread);
    uthread_swap_to_idle();
}

void uthread_unblock(struct uthread_tcb *uthread) {
    // Delete from blocked queue and add to ready queue
    queue_delete(blocked_queue, uthread);
    queue_enqueue(ready_queue, uthread);
}