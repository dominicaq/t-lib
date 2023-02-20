#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

// Tester
// ============================================================================
#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

// Callbacks / Misc functions
// ============================================================================
/* Delete a node that has 42 (Professors example) */
static void iterator_inc(queue_t q, void *data) {
    int *a = (int*)data;

    if (*a == 42)
		queue_delete(q, data);
    else
        *a += 1;
}

/* Print int element of queue */
static void print_int(queue_t q, void *data) {
	int *a = (int*)data;
	if (a != NULL) {
		printf(" [%d]", *(int*)a);
	}
}

/* Print char element of queue */
static void print_char(queue_t q, void *data) {
	char *a = (char*)data;
	if (a != NULL) {
		printf(" [%c]", *(char*)a);
	}
}

/* Print entire queue */
void print_queue(queue_t q) {
	printf("Queue:");
	queue_iterate(q, print_int);
	printf("\n");
}

/* Free entire queue */
void free_queue(queue_t q) {
	int *ptr;
	int queue_len = queue_length(q);
	for (int i = 0; i < queue_len; ++i) {
		queue_dequeue(q, (void**)&ptr);
	}
	queue_destroy(q);
}

// Test functions
// ============================================================================
/* Create a new queue */
void test_create(void) {
	queue_t q = queue_create();

	TEST_ASSERT(q != NULL);
	free_queue(q);
}

/* Test all edge cases of enqueue */
void test_enqueue(void) {
	int data = 3, *ptr;
	queue_t q;

	// Enqueue into uninitalized queue
	TEST_ASSERT(queue_enqueue(q, &data) == -1);

	// Enqueue with null data
	TEST_ASSERT(queue_enqueue(q, NULL) == -1);

	// Enqueue normally
	q = queue_create();
	TEST_ASSERT(queue_enqueue(q, &data) == 0);
}

/* Test all edge cases of dequeue */
void test_dequeue(void) {
	int data = 3, *ptr;
	queue_t q;

	// Dequeue from uninitalized queue
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == -1);
	q = queue_create();
	
	// Dequeue with no target
	TEST_ASSERT(queue_dequeue(q, NULL) == -1);

	// Dequeue from an empty queue
	TEST_ASSERT(queue_dequeue(q, NULL) == -1);

	// Dequeue an item
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

/* Queue length */
void test_len(void) {
	int num_enqueue = 4;
	int num_dequeue = 2;
	int expected_len = num_enqueue - num_dequeue;
	int data = 1;
	queue_t q;

	// Length of uninitalized queue
	TEST_ASSERT(queue_length(q) == -1);
	q = queue_create();

	for (int i = 0; i < num_enqueue; ++i) {
		queue_enqueue(q, &data);
	}

	int *ret;
	for (int i = 0; i < num_dequeue; ++i) {
		queue_dequeue(q, (void**)&ret);
	}

	TEST_ASSERT(queue_length(q) == expected_len);
	free_queue(q);
}

/* Test invalid queue frees */
void test_destroy(void) {
	queue_t q;
	char data[] = {'a', 'b', 'c', 'd', 'e'};
	int data_size = sizeof(data) / sizeof(data[0]);

	// Free uninitalized queue
	TEST_ASSERT(queue_destroy(q) == -1);

	// Init queue
	q = queue_create();
	for (size_t i = 0; i < data_size; i++)
        queue_enqueue(q, &data[i]);

	// Free a non-empty queue
	int *ret;
	for (int i = 0; i < data_size-1; ++i) {
		TEST_ASSERT(queue_destroy(q) == -1);
		queue_dequeue(q, (void**)&ret);
	}

	// Free empty queue
	queue_dequeue(q, (void**)&ret);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Ensure enqueue and deqeue order is correct */
void test_enqueue_dequeue_order(void) {
	queue_t q = queue_create();
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	int data_size = sizeof(data) / sizeof(data[0]);

    /* Enqueue items */
    for (size_t i = 0; i < data_size; i++)
        queue_enqueue(q, &data[i]);

	/* Dequeue and compare ordering */
	for (int i = 0; i < data_size; ++i) {
		int *ret;
		queue_dequeue(q, (void**)&ret);
		TEST_ASSERT(data[i] == *(int*)ret);
	}

	free_queue(q);
}

/* Empty an entire queue with queue_delete only */
void test_delete_node(void) {
	int del_value = 42;
	int data = 41;
	int size = 10;
	queue_t q = queue_create();

	for (int i = 0; i < size; ++i) {
		if (i % 2 == 0) {
			queue_enqueue(q, &del_value);
		} else {
			queue_enqueue(q, &data);
		}
		
	}
	queue_iterate(q, iterator_inc);
	TEST_ASSERT(queue_length(q) == 0);
	free_queue(q);
}

/* Test callbacks (Professors example) */
void test_iterator(void) {
    queue_t q = queue_create();
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};

    /* Initialize the queue and enqueue items */
    for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

	/* Increment every item of the queue, delete item '42' */
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);
	free_queue(q);
}

// TODO: 10-20 test
// Run each test
int main(void) {
	fprintf(stderr, "*** Running queue test ***\n");

	fprintf(stderr, "*** TEST create ***\n");
	test_create();

	fprintf(stderr, "*** TEST enqueue ***\n");
	test_enqueue();

	fprintf(stderr, "*** TEST dequeue ***\n");
	test_dequeue();

	fprintf(stderr, "*** TEST len ***\n");
	test_len();

	fprintf(stderr, "*** TEST enqueue / dequeue order ***\n");
	test_enqueue_dequeue_order();

	fprintf(stderr, "*** TEST queue destroy ***\n");
	test_destroy();


// ------------- Here
	// fprintf(stderr, "*** TEST delete node ***\n");
	// test_free();

	fprintf(stderr, "*** TEST iterator ***\n");
	test_iterator();

	fprintf(stderr, "*** All test passed ***\n");
	return 0;
}
