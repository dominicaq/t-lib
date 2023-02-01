#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#include <stdio.h> // TODO: REMOVE
struct queue {
    int length;
    void *data;
    queue_t front;
    queue_t rear;
};

/*
 * queue_create - Allocate an empty queue
 *
 * Create a new object of type 'struct queue' and return its address.
 *
 * Return: Pointer to new empty queue. NULL in case of failure when allocating
 * the new queue.
 */
queue_t queue_create(void) {
    queue_t new_queue = malloc(sizeof(struct queue));
    if (new_queue == NULL) {
        // ERROR: Bad malloc
        return NULL;
    }
    new_queue->length = 0;
    new_queue->data = NULL;
    new_queue->front = NULL;
    new_queue->rear = NULL;
    return new_queue;
}

/*
 * queue_destroy - Deallocate a queue
 * @queue: Queue to deallocate
 *
 * Deallocate the memory associated to the queue object pointed by @queue.
 *
 * Return: -1 if @queue is NULL or if @queue is not empty. 0 if @queue was
 * successfully destroyed.
 */
int queue_destroy(queue_t queue) {
    if (queue == NULL) {
        // ERROR: Uninitialized queue node
        return -1;
    }

    // Free current queue node
    free(queue);

    if (queue != NULL) {
        // ERROR: Bad free
        return -1;
    }

    return 0;
}

/*
 * queue_enqueue - Enqueue data item
 * @queue: Queue in which to enqueue item
 * @data: Address of data item to enqueue
 *
 * Enqueue the address contained in @data in the queue @queue.
 *
 * Return: -1 if @queue or @data are NULL, or in case of memory allocation error
 * when enqueing. 0 if @data was successfully enqueued in @queue.
 */
int queue_enqueue(queue_t queue, void *data) {
    if (queue == NULL || data == NULL) {
        // ERROR: Uninitialized queue / data
        return -1;
    }

    queue_t new_node = queue_create();
    new_node->data = data;
    if (queue->front == NULL) {
        // Queue is empty
        queue->rear = new_node;
        queue->front = new_node;
    } else {
        new_node->front = queue->rear;
        // Append to rear nodes rear
        queue->rear->rear = new_node;
        queue->rear = new_node;
    }

    queue->length += 1;
    return 0;
}

/*
 * queue_dequeue - Dequeue data item
 * @queue: Queue in which to dequeue item
 * @data: Address of data pointer where item is received
 *
 * Remove the oldest item of queue @queue and assign this item (the value of a
 * pointer) to @data.
 *
 * Return: -1 if @queue or @data are NULL, or if the queue is empty. 0 if @data
 * was set with the oldest item available in @queue.
 */
int queue_dequeue(queue_t queue, void **data) {
    if (queue == NULL || data == NULL) {
        // ERROR: Uninitialized queue / data
        return -1;
    }

    // Get front reference and set data
    queue_t front = queue->front;
    *data = front->data;

    // Make front node equal to front nodes rear
    queue->front = front->rear;
    queue->length -= 1;
    queue_destroy(front);
    return 0;
}

/*
 * queue_delete - Delete data item
 * @queue: Queue in which to delete item
 * @data: Data to delete
 *
 * Find in queue @queue, the first (ie oldest) item equal to @data and delete
 * this item.
 *
 * Return: -1 if @queue or @data are NULL, of if @data was not found in the
 * queue. 0 if @data was found and deleted from @queue.
 */
int queue_delete(queue_t queue, void *data) {
    if (queue == NULL || data == NULL) {
        // ERROR: queue / data is empty
        return -1;
    }

    queue_t q = queue->front;
    while (q != NULL) {
        // Skip logic until data is found
        if (q->data != data) {
            q = q->rear;
            continue;
        }

        // Found target to remove
        if (q->front == NULL) {
            // Target data is in front
            q->rear->front = q->front;
        } else {
            // Data in middle of queue
            q->rear->front = q->front;
            q->front->rear = q->rear;
        }

        queue->length -= 1;
        queue_destroy(q);
        return 0;
    }

    // ERROR: Data not found in queue
    return -1;
}

/*
 * queue_iterate - Iterate through a queue
 * @queue: Queue to iterate through
 * @func: Function to call on each queue item
 *
 * This function iterates through the items in the queue @queue, from the oldest
 * item to the newest item, and calls the given callback function @func on each
 * item. The callback function receives the current data item as parameter.
 *
 * Note that this function should be resistant to data items being deleted
 * as part of the iteration (ie in @func).
 *
 * Return: -1 if @queue or @func are NULL, 0 otherwise.
 */
int queue_iterate(queue_t queue, queue_func_t func) {
    if (queue == NULL || func == NULL) {
        // ERROR: Unintialized queue / func
        return -1;
    }

    // Iterate through queue and use func on current nodes data
    queue_t q = queue->front;
    while (q != NULL) {
        func(queue, q->data);
        q = q->rear;
    }
    
    return 0;
}

int queue_length(queue_t queue) {
    if (queue == NULL) {
        // ERROR: Uninitialized queue
        return -1;
    }
    return queue->length;
}