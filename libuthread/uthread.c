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
#define READY	0
#define RUNNING 1
#define BLOCKED	2
#define ZOMBIE  3

// Thread struct
// =============================================================================
typedef struct uthread_tcb {
	int 		state; // Current thread state
	uthread_ctx_t ctx; // Thread context
	void  *stack_head; // Stack
}uthread_tcb;

// Scheduler
// =============================================================================
queue_t 	READY_QUEUE;
// queue_t 	BLOCKED_QUEUE;
uthread_tcb *CURRENT_THREAD;

struct uthread_tcb *uthread_current(void) {
	// Said this was for the system?
	/* TODO Phase 2/3 */
	return CURRENT_THREAD;
}

/*
 * uthread_yield - Yield execution
 *
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void) {
	// TODO: Check if there are multiple threads
	// Get previous thread
	uthread_tcb *prev_thread = CURRENT_THREAD;
	prev_thread->state = READY;
	// Add current thread to ready queue
	// TODO: This lets thread 1-2 run but prevents 3 from running
	queue_enqueue(READY_QUEUE, CURRENT_THREAD);

	// Get next running thread
	queue_dequeue(READY_QUEUE, (void**)&CURRENT_THREAD);
	CURRENT_THREAD->state = RUNNING;

	// Switch context between threads
	uthread_ctx_switch(&(prev_thread->ctx), &(CURRENT_THREAD->ctx));
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
	/* TODO Phase 2 */
	CURRENT_THREAD->state = ZOMBIE;
	uthread_ctx_destroy_stack(CURRENT_THREAD->stack_head);
	free(CURRENT_THREAD);
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
	// BLOCKED_QUEUE = queue_create();
	
	// Create initial thread
	int retval = uthread_create(func, arg);
	if (retval <= -1) {
		// ERROR: Thread creation failed
		return -1;
	}

	// Set the initial thread
	queue_dequeue(READY_QUEUE, (void**)&CURRENT_THREAD);
	CURRENT_THREAD->state = RUNNING;

	// Execute infinite loop for idle thread
	while (1) {
		// Go to next thread in ready queue
		int num_threads = queue_length(READY_QUEUE);
		if (CURRENT_THREAD->state == RUNNING) {
			func(arg);
		}
		
		if (num_threads == 0) {
			// No more threads to run
			return 0;
		}
	}
}

void uthread_block(void)
{
	/* TODO Phase 3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 3 */
}

