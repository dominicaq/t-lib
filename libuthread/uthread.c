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
// ============================================================================
// TODO: More than likely too many states
#define READY	0
#define BLOCKED	1
#define RUNNING 2
#define ZOMBIE 	3
#define IDLE	4
#define EXITED  5
/* State Notes
* When a thread leaves the ready queue, its running
* Running state -> Blocked || Zombie || Ready
* Blocked State -> Ready
* Zombie State  -> Process Collection
* Ready State 	-> Process Created
* Idle State 	-> In a infinite loop, changes when exited loop
*/

// Thread struct
// ============================================================================
typedef struct uthread_tcb {
	int state;		   // Current thread state
	uthread_ctx_t ctx; // Thread context
	void *stack_head;  // Stack
}uthread_tcb;

// Scheduler
// ============================================================================
queue_t 	READY_QUEUE;
queue_t 	BLOCKED_QUEUE;
uthread_tcb *CURR_THREAD;

struct uthread_tcb *uthread_current(void)
{
	/* TODO Phase 2/3 */
}

/*
 * uthread_yield - Yield execution
 *
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void) {
	/* TODO Phase 2 */
	CURR_THREAD->state = BLOCKED;
	queue_enqueue(BLOCKED_QUEUE, CURR_THREAD);
}

/*
 * uthread_exit - Exit from currently running thread
 *
 * This function is to be called from the currently active and running thread in
 * order to finish its execution.
 *
 * This function shall never return.
 */
void uthread_exit(void)
{
	/* TODO Phase 2 */
	// Kill the CURR_THREAD
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

	// Initialize thread
	new_thread->state = READY;
	new_thread->stack_head = uthread_ctx_alloc_stack();
	int retval = uthread_ctx_init(&new_thread->ctx, new_thread->stack_head, func, arg);
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
	// Set current processes single thread as idle
	// to schedule later for execution
	CURR_THREAD->state = IDLE;
	queue_enqueue(BLOCKED_QUEUE, CURR_THREAD);
	
	// Create a new thread (initial_thread)
	int retval = uthread_create(func, arg);
	if (retval <= -1) {
		// ERROR: Thread creation failed
		return -1;
	}

	// Execute a infinite loop
	while (1) {
		// - when there are no more threads ready to run, stop the idle loop and return
		int len = queue_length(READY_QUEUE)
		if (len == 0) {
			return 0;
		} else {
			// - or yield to next available thread
		}
		
		
		// - or deal with threads that reach completion and destroys their TCB
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

