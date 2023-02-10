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
queue_t 	READY_QUEUE;
queue_t 	BLOCKED_QUEUE;
queue_t     ZOMBIE_QUEUE;
uthread_tcb *CURRENT_THREAD = NULL;
static uthread_ctx_t IDLE_CTX;



struct uthread_tcb *uthread_current(void) {
    return CURRENT_THREAD;
}

/* Free all zombie threads */
void uthread_free(void) {
    // Free zombie threads
    uthread_tcb *target;
    while (queue_dequeue(ZOMBIE_QUEUE, (void**)&target) != -1) {
        uthread_ctx_destroy_stack(target->stack_head);
        free(target);
    }

    // Free Global queues
    queue_destroy(ZOMBIE_QUEUE);
    queue_destroy(BLOCKED_QUEUE);
    queue_destroy(READY_QUEUE);
}

void uthread_yield(void) {
    queue_enqueue(READY_QUEUE, CURRENT_THREAD);
    uthread_ctx_switch(&(CURRENT_THREAD->ctx), &(IDLE_CTX));
}

void uthread_exit(void) {
    queue_enqueue(ZOMBIE_QUEUE, CURRENT_THREAD);
    uthread_ctx_switch(&(CURRENT_THREAD->ctx), &(IDLE_CTX));
}

int uthread_create(uthread_func_t func, void *arg) {
    uthread_tcb *new_thread = malloc(sizeof(uthread_tcb));
    if (new_thread == NULL) {
        // ERROR: Bad malloc
        return -1;
    }

    // Initialize new thread
    new_thread->stack_head = uthread_ctx_alloc_stack();
    int retval = uthread_ctx_init(&(new_thread->ctx), new_thread->stack_head, func, arg);
    if (retval <= -1) {
        // ERROR: Init context failed
        return -1;
    }

    queue_enqueue(READY_QUEUE, new_thread);
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    // Init scheduler
    READY_QUEUE   = queue_create();
    BLOCKED_QUEUE = queue_create();
    ZOMBIE_QUEUE  = queue_create();
    int retval;

    // // Create the user created thread
    retval = uthread_create(func, arg);
    if (retval <= -1) {
        // ERROR: Thread creation failed
        return -1;
    }

    // Idle loop
    while (1) {
        // Exit the idle loop when ready queue is empty
        int num_threads = queue_length(READY_QUEUE);
        if (num_threads == 0) {
            uthread_free();
            return 0;
        }

        // Iniital swap to the next context
        queue_dequeue(READY_QUEUE, (void**)&CURRENT_THREAD);
        uthread_ctx_switch(&(IDLE_CTX), &(CURRENT_THREAD->ctx));
    }
}

void uthread_block(void) {
    // Block the current thread
    queue_enqueue(BLOCKED_QUEUE, CURRENT_THREAD);
    uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread) {
    // Delete from the blocked queue
    queue_delete(BLOCKED_QUEUE, uthread);

    // Add to the ready queue
    queue_enqueue(READY_QUEUE, uthread);
}