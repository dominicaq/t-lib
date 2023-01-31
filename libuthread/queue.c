#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

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
	struct queue new_queue;
	new_queue.length = 0;
	new_queue.data = NULL;
	new_queue.front = NULL;
	new_queue.rear = NULL;
	return &new_queue;
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
		// ERROR: queue pointing to nothing
		return -1;
	}

	// Free queue node
	free(queue);

	if (queue != NULL) {
		// ERROR: Didn't free node
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
		queue->front = queue->rear;
	} else {
		// Append to rear of queue
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
	*data = queue->data;

	// Pop from queue
	queue->front = queue->rear;
	queue->length -= 1;

	// Free front node
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
// O(N)
int queue_delete(queue_t queue, void *data) {
	if (queue == NULL || data == NULL) {
		// ERROR: queue or data is empty
		return -1;
	}

	// Find data in queue
	for (queue_t q = queue; q->front != NULL; q->front = q->front->rear) {
		if (q->data == data) {
			queue_destroy(q);
			return 0;
		}
	}
	
	// ERROR: Did not find data in queue
	return -1;
}

// O(N)
int queue_iterate(queue_t queue, queue_func_t func) {
	return -1;
	/* TODO Phase 1 */
}

int queue_length(queue_t queue) {
	if (queue == NULL) {
		// ERROR: Uninitialized queue
		return -1;
	}
	return queue->length;
}