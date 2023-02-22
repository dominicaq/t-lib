#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

// Tester
// ============================================================================
#define TEST_ASSERT(assert)             \
do {                                    \
    printf("ASSERT: " #assert " ... "); \
    if (assert) {                       \
        printf("PASS\n");               \
    } else  {                           \
        printf("FAIL\n");               \
        exit(1);                        \
    }                                   \
} while(0)

// Callbacks / Misc functions
// ============================================================================
/* Print int elements and increment them by 10 */
static void print_int_increment(queue_t q, void *data) {
    int *a = (int*)data;
    if (a != NULL) {
        *a += 10;
        printf("[%d] ", *(int*)a);
    }
}

/* Print int element of queue */
static void print_int(queue_t q, void *data) {
    int *a = (int*)data;
    if (a != NULL) {
        printf("[%d] ", *(int*)a);
    }
}

/* Print char element of queue */
static void print_char(queue_t q, void *data) {
    char *a = (char*)data;
    if (a != NULL) {
        printf("[%c] ", *(char*)a);
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
    if (q == NULL) {
        return;
    }

    int *data;
    while (!queue_dequeue(q, (void**)&data));
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
    queue_t q = NULL;

    // Enqueue into uninitalized queue
    TEST_ASSERT(queue_enqueue(q, &data) == -1);
    q = queue_create();

    // Enqueue with null data
    TEST_ASSERT(queue_enqueue(q, NULL) == -1);

    // Enqueue normally
    TEST_ASSERT(queue_enqueue(q, &data) == 0);

    // Free test
    free_queue(q);
}

/* Test all edge cases of dequeue */
void test_dequeue(void) {
    int data = 3, *ptr;
    queue_t q = NULL;

    // Dequeue from uninitalized queue
    TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == -1);
    q = queue_create();

    // Dequeue with no target
    TEST_ASSERT(queue_dequeue(q, NULL) == -1);

    // Dequeue from an empty queue
    TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == -1);

    // Dequeue an item
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data);

    // Free test
    free_queue(q);
}

/* Queue length */
void test_len(void) {
    int num_enqueue = 4;
    int num_dequeue = 2;
    int expected_len = num_enqueue - num_dequeue;
    int data = 1;
    queue_t q = NULL;

    // Length of uninitalized queue
    TEST_ASSERT(queue_length(q) == -1);
    q = queue_create();

    // Initial length of empty queue
    TEST_ASSERT(queue_length(q) == 0);

    // Enqueue data
    for (int i = 0; i < num_enqueue; ++i) {
        queue_enqueue(q, &data);
    }
    TEST_ASSERT(queue_length(q) == num_enqueue);

    // Dequeue some data
    int *ret;
    for (int i = 0; i < num_dequeue; ++i) {
        queue_dequeue(q, (void**)&ret);
    }
    TEST_ASSERT(queue_length(q) == expected_len);

    // Dequeue the rest of the data
    while(queue_length(q)) {
        queue_dequeue(q, (void**)&ret);
    }
    TEST_ASSERT(queue_length(q) == 0);

    // Free test
    free_queue(q);
}

/* Ensure enqueue and deqeue order is correct */
void test_enqueue_dequeue_order(void) {
    queue_t q = queue_create();
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    int data_size = sizeof(data) / sizeof(data[0]);

    // Enqueue items
    for (size_t i = 0; i < data_size; i++) {
        queue_enqueue(q, &data[i]);
    }

    // Dequeue and compare ordering
    for (int i = 0; i < data_size; ++i) {
        int *ret;
        queue_dequeue(q, (void**)&ret);
        TEST_ASSERT(data[i] == *(int*)ret);
    }

    // Enqueue some more items and check length
    //   Tests enqueueing from a dequeue-emptied queue
    for (size_t i = 0; i < data_size; i++) {
        queue_enqueue(q, &data[i]);
    }
    TEST_ASSERT(queue_length(q) == data_size);

    // Free test
    free_queue(q);
}

/* Test invalid queue frees */
void test_destroy(void) {
    queue_t q = NULL;
    char data[] = {'a', 'b', 'c', 'd', 'e'};
    int data_size = sizeof(data) / sizeof(data[0]);

    // Free uninitalized queue
    TEST_ASSERT(queue_destroy(q) == -1);

    // Init queue
    q = queue_create();
    for (size_t i = 0; i < data_size; i++) {
        queue_enqueue(q, &data[i]);
    }

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

/* Empty an entire queue with queue_delete only */
void test_delete_node(void) {
    int dataHead=0, data1=1, data2=2, data3=3, dataTail=4;
    int numData = 20;
    queue_t q = NULL;

    // Delete a item from a unintalized queue
    TEST_ASSERT(queue_delete(q, (void*)&data1) == -1);
    q = queue_create();

    // Delete a NULL item from a initalized queue
    TEST_ASSERT(queue_delete(q, NULL) == -1);

    // Insert equal number of data1 and data2
    //   bookended by dataHead and dataTail
    queue_enqueue(q, &dataHead);
    for (int i = 0; i < numData; ++i) {
        if ((i&1) == 0) {
            queue_enqueue(q, &data1);
        } else {
            queue_enqueue(q, &data2);
        }
    }
    queue_enqueue(q, &dataTail);

    // Delete an item that doesn't exist
    TEST_ASSERT(queue_delete(q, (void*)&data3) == -1);

    // Delete data1 nodes
    while (!queue_delete(q, (void*)&data1));

    // Test for half-empty queue
    TEST_ASSERT(queue_length(q) == numData/2 + 2);

    // Delete head and tail
    queue_delete(q, (void*)&dataHead);
    queue_delete(q, (void*)&dataTail);
    TEST_ASSERT(queue_length(q) == numData/2);

    // Enqueue some more nodes, to ensure stability of queue
    for (int i = 0; i < numData; ++i) {
        queue_enqueue(q, &data2);
    }
    TEST_ASSERT(queue_length(q) == numData + numData/2);

    // Dequeue the rest, to ensure stability of queue
    int *ptr;
    while (!queue_dequeue(q, (void**)&ptr));

    // Test for empty queue
    TEST_ASSERT(queue_length(q) == 0);
    free_queue(q);
}

/* Test callbacks */
void test_iterator(void) {
    queue_t q_blank = NULL;

    // Iterate over unintalized queue
    TEST_ASSERT(queue_iterate(q_blank, print_char) == -1);
    q_blank = queue_create();

    // Iterate over queue with a NULL func
    TEST_ASSERT(queue_iterate(q_blank, NULL) == -1);

    queue_t q_int = queue_create();
    queue_t q_char = queue_create();
    int data_int[] = {1, 2, 3, 4, 5, 2, 6, 7, 8, 9};
    int data_char[] = {'a', 'b', 'c', 'd', 'e', 'f'};

    // Enqueue int items
    for (size_t i = 0; i < sizeof(data_int) / sizeof(data_int[0]); i++)
        queue_enqueue(q_int, &data_int[i]);

    // Enqueue char items
    for (size_t i = 0; i < sizeof(data_char) / sizeof(data_char[0]); i++)
        queue_enqueue(q_char, &data_char[i]);

    // Print entire queue with iterate
    queue_iterate(q_char, print_char);
    printf("\n");
    queue_iterate(q_int, print_int_increment);
    printf("\n");

    TEST_ASSERT(data_int[0] == 11);
    TEST_ASSERT(data_char[3] == 'd');
    free_queue(q_int);
    free_queue(q_char);
    free_queue(q_blank);
}

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

    fprintf(stderr, "*** TEST queue delete node ***\n");
    test_delete_node();

    fprintf(stderr, "*** TEST iterator ***\n");
    test_iterator();

    fprintf(stderr, "*** All test passed ***\n");
    return 0;
}
