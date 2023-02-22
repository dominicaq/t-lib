# LIBUTHREAD: User-level thread library
`libuthread` is a library for creating and scheduling threads in user-space.
Along with the thread API, the library provides a linked-list queue
implementation as well a simple semaphore implementation. The thread library
supports scheduling via yielding or it can be conditionally initialized with
preemptive scheduling. The sempahore library is thread-safe with preemption, but
the queue library is not.

## Usage
The `libuthread` library includes a `Makefile` to build a binary object file to
link with your program. This can be done as follows:
```bash
$ make -C path/to/libuthread
```
This will make the object file `libuthread.a` in the `libuthread` directory,
which can then used for linking.

## Features
- User-space thread library with configurable scheduling
  - Fully-controlled yield scheduling
  - Automatically preemptive round-robin scheduling
- Fully generic and non-owning queue library with linked-list structures
  - Supports functional iterators and search deletions with pointers as keys
- Semaphore library designed around the thread library
  - Works out-of-the-box with the preemptive scheduling
  - Blocks threads that fail acquiring the semaphore to prevent wasted cycles

## Queue Library
The `queue` struct is built as a wrapper around a doubley-linked-list of `node`
structs. `queue` has a field for its length as well as references to the head
and tail nodes (`head` and `tail` are set to `NULL` if and only if `length==0`).

Each `node` simply holds references to its previous and next neighbors (`next`
goes towards the tail and represents `nodes` enqueued after this `node`) as
well as a reference pointer to the `data` which the node holds. The `node` does
not own the data and the queue will never free any `data` pointers. This `data`
pointer is passed back to the caller upon dequeueing this node and it can also
be used as a key in `queue_delete` operations.

The queue supports enqueuing and dequeueing, as expected, but it also supports iterating over the queue. `queue_iterate` takes a `queue` and a function of type
`func(queue *q, void *data)` which will call `func` on every element of the
queue. Because the queue itself is passed to the function, any queue function
can be called within `func`. This includes `queue_delete`, as the iterator has
been designed to be resilient to deletions of the current node. That being said,
if the `next` node is deleted or dequeued from `funct` then the iterator will
fail by attempting to access that freed node.

### Testing the Queue Library
We provided a queue testing program in `queue_tester.c` with test cases that
are designed to reach every code-point in the `queue.c` library source file.
Along with this, we ensure that every novel state-change of the queue (emptied
after having elements, getting head or tail deleted, etc). Through this, we
believe that our queue is stable in every situation except the one described
above.

## Thread Library
The thread library is exposed in the file `uthread.h`. From here, the initial
scheduling execution can be entered with `uthread_run` which also runs the
initial thread. Subsequent threads can be scheduled by calling `uthread_create`
from the running thread function. Both `uthread_run` and `uthread_create` take a
thread body function and a list of arguments to pass to the function.
`uthread_run` also takes a flag to conditionally use preemptive scheduling.

### Thread Struct
The thread struct `uthread_tcb` holds the context of the thread as well as a
reference to the head of its stack (for freeing upon destruction). This context
holds all of the relevant information for the thread to run, such as its stack
pointer, program counter, and data register values.

### Thread Lifetime
When a thread is created, its struct is initialized and it is enqueued into the
library's ready queue. The ready queue is used for scheduling and holds all
actively running threads that are not blocked.

A thread may be dequeued from the ready queue at any time and get activated
by having its context switched to by another thread in the running process. This
thread will continue to run into it yields, is blocked, is preempted, or it
completes execution of its function and exits.

If a thread is yielded or preempted, it is enqueued back onto the ready queue
before getting its context switched away from, freezing its state. The running
thread may also be blocked, where it waits until it is specifically
unblocked with a reference to its thread struct. When a thread exits, it is
added to a zombie queue where the idle thread later frees its memory.

### Context Switching
Context switching is the act of atomically swapping register information from
one context to another. This register information importantly includes the stack
pointer and program counter, which is what defines the execution of a program.
Threads in our library switch from one thread to the next upon yielding or
being preempted. There is no kernel-like process handling the scheduling.

### Idle Thread
The idle thread is launched by `uthread_run` and it executes a specialized
program defined by our thread library. It is enqueued and switched to like every
other user thread. The idle thread has two purposes. First, whenever the idle
thread becomes the running process it first checks to see if there are any
zombie threads, destroying them if so. Second, it checks if there are any other
threads being run by the scheduler. If there are no other threads, the idle
thread cleans up the scheduling structures and exits `uthread_run`, returning
control to the caller of `uthread_run`.

### Yield Scheduling
The default scheduling mechanism is to provide full scheduling control to the
thread's passed execution functions through the `uthread_yield` function. This
function causes the currently running thread to be returned to the ready queue
before being switched for the next running thread.

This scheduling strategy gives full control to the user, but if any thread does
not yield, it will not give concurrent access to any other thread until it has
finished execution or has been blocked by a semaphore.

### Preemptive Scheduling
The user can also intiate the thread library with preemptive scheduling. This
forcibly yields the current thread after some amount of time passes from a
virtual timer (that only checks process running time). This timer signal is
internally blocked for specific critical sections that affect shared scheduling
data. Namely, any modifications to the ready, blocked, and zombie queues are
atomic with respect to preemption. Since `swapcontext` is a syscall, it is
implicitly atomic as the virtual timer can not interrupt the kernel's actions.

### Testing Preemptive Scheduling
The testing program found in `test_preempt.c` is used to ensure execution of a
resource-hogging thread will be halted by the virtual timer. In order to achieve
this, we create a cascade of 4 threads with each thread being created by the
previous one. The first 3 threads are stuck in loops dependent on the value of a
global int called `INFINITE_LOOP`. This value is initialized to `1` and each of
these first 3 threads will loop forever while this value remains `1`.

Each thread prints a statement upon first entering its running function,
signaling that the thread has been reached. Since there is no yielding,
if preemption failed entirely then only the first thread would print before
entering its infinite loop, never yielding the process to the other threads.
However, we can see that each thread prints its statement as the infinite
loops are getting interrupted by the timer signal, yielding the threads.

Finally, once the 4th thread begins execution, it prints and then sets
`INFINITE_LOOP` to 0, forcing the other threads to exit once control is returned
to them. This works because, although the value of `INFINITE_LOOP` would often
be cached in a register for these threads, we mark `INFINITE_LOOP` as volatile
to prevent this caching and ensure that the threads always read the latest
value. The 4th thread exits and this change causes a cascade down where each of
the looping threads exit their loops and thus exit their executing fucntions.
This leads to all 4 threads completing and the program ends, showing our
preemption works in at least this simple case.

## Semaphore Library


### Blocking Threads

### Unblocking Threads