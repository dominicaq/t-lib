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

// Swap threads from current thread to new thread
void uthread_swap_threads(void) {
    if (queue_length(ready_queue) == 0) {
        return;
    }

    // Retrieve next ready thread (atomic)
    preempt_disable();
    uthread_tcb *prev_thread = current_thread;
    queue_dequeue(ready_queue, (void**)&current_thread);
    preempt_enable();

    // Switch context
    uthread_ctx_switch(&(prev_thread->ctx), &(current_thread->ctx));
}

void uthread_yield(void) {
    // Enqueue current thread into ready queue (atomic)
    preempt_disable();
    queue_enqueue(ready_queue, current_thread);
    preempt_enable();

    // Swap to next ready thread
    uthread_swap_threads();
}

void uthread_exit(void) {
    // Enqueue current thread into zombie queue (atomic)
    preempt_disable();
    queue_enqueue(zombie_queue, current_thread);
    preempt_enable();

    // Swap to next ready thread
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

    int retval = uthread_ctx_init(&(new_thread->ctx), new_thread->stack_head, 
        func, arg);
    if (retval <= -1) {
        // ERROR: Init context failed
        return -1;
    }

    // Enqueue current thread into ready queue (atomic)
    preempt_disable();
    queue_enqueue(ready_queue, new_thread);
    preempt_enable();

    return 0;
}

// Empty out a queue and free tcb mallocs
void uthread_free_queue(queue_t target_queue) {
    // Disable preempt, entering critical section
    preempt_disable();

    while (queue_length(target_queue) > 0) {
        uthread_tcb* target_thread;
        queue_dequeue(target_queue, (void**)&target_thread);
        if (target_thread != NULL) {
            uthread_ctx_destroy_stack(target_thread->stack_head);
            free(target_thread);
        }
    }

    // Reenable preempt, exiting critical section
    preempt_enable();
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

    // Preemption init
    preempt_start(preempt);

    // Idle loop
    while (1) {
        // Exit the idle loop when ready queue is empty
        int num_threads = queue_length(ready_queue);
        if (num_threads == 0) {
            break;
        }
        
        // Swap to the next thread
        uthread_yield();

        // Free zombies in idle loop 
        uthread_free_queue(zombie_queue);
    }

    // Stop preemption
    preempt_disable();
    preempt_stop();

    // Free current thread
    uthread_ctx_destroy_stack(current_thread->stack_head);
    free(current_thread);

    // Destroy queues
    queue_destroy(blocked_queue);
    queue_destroy(zombie_queue);
    queue_destroy(ready_queue);
    return 0;
}

// Block the current thread
void uthread_block(void) {
    // Enqueue current thread into blocked queue (atomic)
    preempt_disable();
    queue_enqueue(blocked_queue, current_thread);
    preempt_enable();

    // Yield blocked thread
    uthread_yield();
}

// Unblock a target thread
void uthread_unblock(struct uthread_tcb *uthread) {
    // Disable preempt, entering critical section
    preempt_disable();

    // Delete from blocked queue and add to ready queue if it existed
    int retval = queue_delete(blocked_queue, uthread);
    if (retval > 0) {
        queue_enqueue(ready_queue, uthread);
    }

    // Reenable preempt, exiting critical section
    preempt_enable();
}