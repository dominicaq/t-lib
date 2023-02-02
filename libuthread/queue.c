#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue {
    int capacity; // Must be power of 2
    int head;
    int length;
    void **ptr;
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
    queue_t new_queue = malloc(sizeof(queue_t));
    if (new_queue == NULL) {
        // ERROR: Bad malloc
        return NULL;
    }
    new_queue->capacity = 8;
    new_queue->ptr = malloc(new_queue->capacity);
    new_queue->head = 0;
    new_queue->length = 0;
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
        // ERROR: Uninitalized queue
        return -1;
    }

    // Unsure how to free **ptr in a linear way
    free(queue);
    
    if (queue != NULL) {
        // ERROR: Bad malloc
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
    // Double capcity if it has been reached
    if (queue->length == queue->capacity) {
        queue->capacity *= 2;
    }

    int index = (queue->head + queue->length) & (queue->capacity -1);
    queue->ptr[index] = data;
    ++queue->length;

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
    if (queue == NULL || queue->length == 0 || data == NULL) {
        // ERROR: Empty queue / data
        return -1;
    }
    
    // Set data to head data;
    data = queue->ptr[queue->head];
    // Move head index
    queue->head = (queue->head + 1) & (queue->capacity - 1);
    --queue->length;
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

    int index = queue->head;
    while (index != queue->length) {
        if (queue->ptr[index] != data) {
            ++index;
            continue;
        }
        
        // Remove target data
        // - Free it
        --queue->length;
        return 0;
    }

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

    int curr_index = queue->head;
    while (curr_index != queue->length) {
        func(queue, queue->ptr[curr_index]);
        ++curr_index;
    }

    return 0;
}

int queue_length(queue_t queue) {
    if (queue == NULL) {
        // ERROR: Uninitalized queue
        return -1;
    }

    return queue->length;
}