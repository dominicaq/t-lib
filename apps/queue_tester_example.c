#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

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

/* Callback - Professor code */
static void iterator_inc(queue_t q, void *data)
{
    int *a = (int*)data;

    if (*a == 42)
		queue_delete(q, data);
    else
        *a += 1;
}

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

/* Queue length */
void test_len(void) {
	int expected_len = 4;
	int data = 3;
	queue_t q;

	// Enqueue 4 data points
	q = queue_create();
	for (int i = 0; i < expected_len; ++i) {
		queue_enqueue(q, &data);
	}

	fprintf(stderr, "*** TEST len ***\n");
	TEST_ASSERT(queue_length(q) == expected_len);
}

/* Callback */
void test_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Increment every item of the queue, delete item '42' */
    queue_iterate(q, iterator_inc);
    assert(data[0] == 2);
	printf("ITERATOR_LEN: %d\n", queue_length(q));
    assert(queue_length(q) == 9);
}

int main(void) {
	test_create();
	test_queue_simple();
	test_len();
	test_iterator();

	return 0;
}
