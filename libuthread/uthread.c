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

// Thread states
// =============================================================================
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
    IDLE
} state_e;

// Thread struct
// =============================================================================
typedef struct uthread_tcb {
    state_e     state; // Current thread state
    uthread_ctx_t ctx; // Thread context
    void  *stack_head; // Stack
} uthread_tcb;

// Scheduler
// =============================================================================
queue_t 	READY_QUEUE;
queue_t 	BLOCKED_QUEUE;
queue_t     ZOMBIE_QUEUE;
uthread_tcb *CURRENT_THREAD = NULL;
uthread_tcb *IDLE_THREAD = NULL;

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

/* Handles swaping thread context from current thread to new thread */
void swap_thread_ctx(void) {
    // Make sure theres threads available to swap to
    int num_threads = queue_length(READY_QUEUE);
    if (num_threads == 0) {
        IDLE_THREAD->state = RUNNING;
        uthread_ctx_switch(&(CURRENT_THREAD->ctx), &(IDLE_THREAD->ctx));
        return;
    }

    uthread_tcb *prev_thread = CURRENT_THREAD;

    // Get next running thread
    queue_dequeue(READY_QUEUE, (void**)&CURRENT_THREAD);

    // Skip swapping if next thread is the same as prev
    if (prev_thread == CURRENT_THREAD) {
        return;
    }

    // Switch context between threads
    CURRENT_THREAD->state = RUNNING;
    uthread_ctx_switch(&(prev_thread->ctx), &(CURRENT_THREAD->ctx));
}

/*
 * uthread_yield - Yield execution
 *
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void) {
    CURRENT_THREAD->state = READY;
    queue_enqueue(READY_QUEUE, CURRENT_THREAD);
    swap_thread_ctx();
}

/*
 * uthread_exit - Exit from currently running thread
 *
 * This function is to be called from the currently active and running thread in
 * order to finish its execution.
 *
 * This function shall never return.
 */
void uthread_exit(void) {
    CURRENT_THREAD->state = ZOMBIE;
    queue_enqueue(ZOMBIE_QUEUE, CURRENT_THREAD);
    swap_thread_ctx();
}

/*
 * uthread_create - Create a new thread
 * @func: Function to be executed by the thread
 * @arg: Argument to be passed to the thread
 *
 * This function creates a new thread running the function @func to which
 * argument @arg is passed.
 *
 * Return: 0 in case of success, -1 in case of failure (e.g., memory allocation,
 * context creation).
 */
int uthread_create(uthread_func_t func, void *arg) {
    uthread_tcb *new_thread = malloc(sizeof(uthread_tcb));
    if (new_thread == NULL) {
        // ERROR: Bad malloc
        return -1;
    }

    // Initialize new thread
    new_thread->state = READY;
    new_thread->stack_head = uthread_ctx_alloc_stack();
    int retval = uthread_ctx_init(&(new_thread->ctx), new_thread->stack_head, func, arg);
    if (retval <= -1) {
        // ERROR: Init context failed
        return -1;
    }

    queue_enqueue(READY_QUEUE, new_thread);
    return 0;
}

/*
 * uthread_run - Run the multithreading library
 * @preempt: Preemption enable
 * @func: Function of the first thread to start
 * @arg: Argument to be passed to the first thread
 *
 * This function should only be called by the process' original execution
 * thread. It starts the multithreading scheduling library, and becomes the
 * "idle" thread. It returns once all the threads have finished running.
 *
 * If @preempt is `true`, then preemptive scheduling is enabled.
 *
 * Return: 0 in case of success, -1 in case of failure (e.g., memory allocation,
 * context creation).
 */
int uthread_run(bool preempt, uthread_func_t func, void *arg) {
    // Init scheduler
    READY_QUEUE   = queue_create();
    BLOCKED_QUEUE = queue_create();
    ZOMBIE_QUEUE  = queue_create();
    int retval;

    // Create idle thread
    retval = uthread_create(NULL, NULL);
    if (retval <= -1) {
        // ERROR: Thread creation failed
        return -1;
    }
    queue_dequeue(READY_QUEUE, (void**)&IDLE_THREAD);
    IDLE_THREAD->state = IDLE;

    // // Create the user created thread
    retval = uthread_create(func, arg);
    if (retval <= -1) {
        // ERROR: Thread creation failed
        return -1;
    }
    queue_dequeue(READY_QUEUE, (void**)&CURRENT_THREAD);
    CURRENT_THREAD->state = RUNNING;

    // Iniital swap to the next context
    uthread_ctx_switch(&(IDLE_THREAD->ctx), &(CURRENT_THREAD->ctx));

    // Idle loop
    while (1) {
        // Exit the idle loop when ready queue is empty
        int num_threads = queue_length(READY_QUEUE);
        if (num_threads == 0) {
            queue_enqueue(ZOMBIE_QUEUE, IDLE_THREAD);
            uthread_free();
            return 0;
        }
    }
}

void uthread_block(void) {
    // Block the current thread
    CURRENT_THREAD->state = BLOCKED;
    queue_enqueue(BLOCKED_QUEUE, CURRENT_THREAD);

    // Yield to next available thread
    swap_thread_ctx();
}

void uthread_unblock(struct uthread_tcb *uthread) {
    // Delete from the blocked queue
    queue_delete(BLOCKED_QUEUE, uthread);

    // Add to the ready queue
    uthread->state = READY;
    queue_enqueue(READY_QUEUE, uthread);
}

