#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

#include "uthread.h"

// All functions need to be complete for phase 3
/*
 * sem_t - Semaphore type
 *
 * A semaphore is a way to control access to a common resource by multiple
 * threads. Such resource has an internal count, meaning that it can only be
 * shared a certain number of times. When a thread successfully takes the
 * resource, the count is decreased. When the resource is not available,
 * following threads are blocked until the resource becomes available again.
 */
struct semaphore {
    size_t count;
    int lock;
    queue_t waiting_queue;
};

/*
 * sem_create - Create semaphore
 * @count: Semaphore count
 *
 * Allocate and initialize a semaphore of internal count @count.
 *
 * Return: Pointer to initialized semaphore. NULL in case of failure when
 * allocating the new semaphore.
 */
sem_t sem_create(size_t count) {
    sem_t new_sem = malloc(sizeof(struct semaphore));
    if (new_sem == NULL) {
        // ERROR: Bad malloc 
        return NULL;
    }

    new_sem->waiting_queue = queue_create();
    new_sem->count = count;
    new_sem->lock = 0;
    return new_sem;
}

/*
 * sem_destroy - Deallocate a semaphore
 * @sem: Semaphore to deallocate
 *
 * Deallocate semaphore @sem.
 *
 * Return: -1 if @sem is NULL or if other threads are still being blocked on
 * @sem. 0 is @sem was successfully destroyed.
 */
int sem_destroy(sem_t sem) {
    if (sem == NULL || queue_length(sem->waiting_queue) > 0) {
        // ERROR: Bad sem destroy
        return -1;
    }

    // Free waiting queue
    queue_destroy(sem->waiting_queue);
    free(sem);
    return 0;
}

/*
 * sem_down - Take a semaphore
 * @sem: Semaphore to take
 *
 * Take a resource from semaphore @sem.
 *
 * Taking an unavailable semaphore will cause the caller thread to be blocked
 * until the semaphore becomes available.
 *
 * Return: -1 if @sem is NULL. 0 if semaphore was successfully taken.
 */
int sem_down(sem_t sem) {
    if (sem == NULL) {
        // ERROR: Uninitalized sem
        return -1;
    }

    while (sem->count == 0) {
        struct uthread_tcb *current_thread = uthread_current();
        queue_enqueue(sem->waiting_queue, current_thread);
        uthread_block();
    }

    --(sem->count);
    return 0;
}


/*
 * sem_up - Release a semaphore
 * @sem: Semaphore to release
 *
 * Release a resource to semaphore @sem.
 *
 * If the waiting list associated to @sem is not empty, releasing a resource
 * also causes the first thread (i.e. the oldest) in the waiting list to be
 * unblocked.
 *
 * Return: -1 if @sem is NULL. 0 if semaphore was successfully released.
 */
int sem_up(sem_t sem) {
    if (sem == NULL) {
        // ERROR: Uninitalized sem
        return -1;
    }

    ++(sem->count);

    // Wake up first in line if any
    int num_threads = queue_length(sem->waiting_queue);
    if (num_threads > 0) {
        struct uthread_tcb *unblocked_thread;
        queue_dequeue(sem->waiting_queue, (void**)&unblocked_thread);
        uthread_unblock(unblocked_thread);
    }

    return 0;
}
