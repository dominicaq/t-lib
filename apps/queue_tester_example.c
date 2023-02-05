#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

// TODO LIST:
// - More test functions
// - Check invalid queue frees

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
/* Create */
void test_create(void) {
	queue_t q = queue_create();

	TEST_ASSERT(q != NULL);
	free_queue(q);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void) {
	int data = 3, *ptr;
	queue_t q = queue_create();

	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
	free_queue(q);
}

/* Queue length */
void test_len(void) {
	int num_enqueue = 4;
	int num_dequeue = 2;
	int expected_len = num_enqueue - num_dequeue;
	int data = 1;
	queue_t q = queue_create();

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
void test_free(void) {
	queue_t q;
	char data[] = {'a', 'b', 'c', 'd', 'e'};
	int data_size = sizeof(data) / sizeof(data[0]);

	// Free uninitalized queue
	TEST_ASSERT(queue_destroy(q) == -1);

	// Init queue
	q = queue_create();
	for (size_t i = 0; i < data_size; i++)
        queue_enqueue(q, &data[i]);

	// Free non-empty queue
	int *ret;
	for (int i = 0; i < data_size-1; ++i) {
		TEST_ASSERT(queue_destroy(q) == -1);
		queue_dequeue(q, (void**)&ret);
	}

	// Free empty queue
	queue_dequeue(q, (void**)&ret);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Ensure enqueue and deqeue are correct */
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
void test_delete(void) {
	int data = 42;
	int size = 10;
	queue_t q = queue_create();

	for (int i = 0; i < size; ++i) {
		queue_enqueue(q, &data);
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

/* Mass insertions / deletes */
void test_rigirous(void){
	queue_t q = queue_create();
	int num_itterations = 1000;
	int data = 24;
	int *ret;

	// Expect no mem leaks
	for (int i = 0; i < num_itterations; ++i) {
		queue_enqueue(q, &data);
		for (int j = 0; j < num_itterations; ++j) {
			queue_dequeue(q, (void**)&ret);
		}
	}
	TEST_ASSERT(queue_length(q) == 0);
	free_queue(q);
}

// Run each test
int main(void) {
	fprintf(stderr, "*** Running queue test ***\n");

	fprintf(stderr, "*** TEST create ***\n");
	test_create();

	fprintf(stderr, "*** TEST queue_simple ***\n");
	test_queue_simple();

	fprintf(stderr, "*** TEST len ***\n");
	test_len();

	fprintf(stderr, "*** TEST enqueue / dequeue order ***\n");
	test_enqueue_dequeue_order();

	fprintf(stderr, "*** TEST free queue ***\n");
	test_free();

	fprintf(stderr, "*** TEST delete ***\n");
	test_delete();

	fprintf(stderr, "*** TEST iterator ***\n");
	test_iterator();

	fprintf(stderr, "*** TEST rigirous ***\n");
	test_rigirous();

	fprintf(stderr, "*** All test passed ***\n");
	return 0;
}
