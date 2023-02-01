#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

// TODO LIST:
// - Edge Cases
// - Freeing after test
// Assert prompt
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

// Callbacks
// ============================================================================
static void iterator_inc(queue_t q, void *data) {
    int *a = (int*)data;

    if (*a == 42)
		queue_delete(q, data);
    else
        *a += 1;
}

static void print_queue(queue_t q, void *data) {
	int *a = (int*)data;
	if (a != NULL) {
		printf("Queue_list: %d\n", *(int*)a);
	}
}

static void free_queue(queue_t q, void *data) {
	queue_destroy((queue_t)data);
}
// Test functions
// ============================================================================
/* Create */
void test_create(void) {
	queue_t q = queue_create();
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(q != NULL);
	queue_destroy(q);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q = queue_create();

	fprintf(stderr, "*** TEST queue_simple ***\n");

	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
	queue_destroy(q);
}

/* Queue length */
void test_len(void) {
	int num_enqueue = 4;
	int num_dequeue = 2;
	int expected_len = num_enqueue - num_dequeue;
	int data = 1;
	queue_t q;

	fprintf(stderr, "*** TEST len ***\n");

	q = queue_create();
	for (int i = 0; i < num_enqueue; ++i) {
		queue_enqueue(q, &data);
	}

	int *ret;
	for (int i = 0; i < num_dequeue; ++i) {
		queue_dequeue(q, (void**)&ret);
	}

	TEST_ASSERT(queue_length(q) == expected_len);
}

/* Test callbacks */
void test_iterator(void) {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

	fprintf(stderr, "*** TEST iterator ***\n");
    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

	/* Increment every item of the queue, delete item '42' */
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);
}

/* Enqueue one data point and remove it with queue_delete
	-Expected result: Empty queue
*/
// TODO: Edge case: making the queue empty causes seg fault
void test_enqueue_delete() {
	int data = 42;
	int data2 = 200;
	queue_t q = queue_create();

	fprintf(stderr, "*** TEST enqueue_delete ***\n");
	queue_enqueue(q, &data);
	queue_enqueue(q, &data);

	queue_iterate(q, iterator_inc);
	// queue_enqueue(q, &data);
	queue_iterate(q, print_queue);
	TEST_ASSERT(queue_length(q) == 0);
}

int main(void) {
	test_create();
	test_queue_simple();
	test_len();
	test_iterator();
	test_enqueue_delete();

	return 0;
}
