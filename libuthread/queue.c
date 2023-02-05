#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node {
    void *data;
    struct node *prev;
    struct node *next;
} node;

struct queue {
    int length;
    node *head;
    node *tail;
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
    new_queue->head = NULL;
    new_queue->tail = NULL;
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
    if (queue == NULL || queue->length > 0) {
        // ERROR: Uninitialized queue node
        return -1;
    }

    // Free current queue node
    free(queue);
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

    // Init new node
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL) {
        // ERROR: Bad malloc
        return -1;
    }

    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->data = data;

    // Add to queue
    if (queue->head == NULL) {
        // Queue is empty
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        new_node->prev = queue->tail;
        // Append to rear nodes rear
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    ++(queue->length);
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
    if (queue == NULL || data == NULL || queue->length == 0) {
        // ERROR: Uninitialized queue / data
        return -1;
    }

    // Get front reference and set data
    node *to_deq = queue->head;
    *data = to_deq->data;

    // Make front node equal to front nodes rear
    queue->head = to_deq->next;
    queue->tail->next = NULL;
    --(queue->length);
    free(to_deq);
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

    // Iterate until target data is found
    node *target = queue->head;
    while (target != NULL && target->data != data) {
        target = target->next;
    }

    // ERROR: Data not found in queue
    if (target == NULL) {
        return -1;
    }
    
    if (queue->head != target && queue->tail != target) {
        // Target is in middle
        target->prev->next = target->next;
        target->next->prev = target->prev;
    } else {
        // Target is on either edge of queue
        if (queue->head == target) {
            // Target is head
            queue->head = target->next;
        }

        if (queue->tail == target) {
            // Target is rear
            queue->tail = target->prev;
        }
    }

    free(target);
    --(queue->length);
    return 0;
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
    node *curr_node = queue->head;
    while (curr_node != NULL) {
        node *next = curr_node->next;
        func(queue, curr_node->data);
        curr_node = next;
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