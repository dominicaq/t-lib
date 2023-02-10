#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

bool USE_PREEMPT;
struct sigaction SA;

void preempt_handler(int signum) {
    uthread_yield();
}

/*
 * preempt_disable - Disable preemption
 */
void preempt_disable(void) {
    USE_PREEMPT = false;
	/* TODO Phase 4 */
}

/*
 * preempt_enable - Enable preemption
 */
void preempt_enable(void) {
    USE_PREEMPT = true;
	/* TODO Phase 4 */
}

/*
 * preempt_start - Start thread preemption
 * @preempt: Enable preemption if true
 *
 * Configure a timer that must fire a virtual alarm at a frequency of 100 Hz and
 * setup a timer handler that forcefully yields the currently running thread.
 *
 * If @preempt is false, don't start preemption; all the other functions from
 * the preemption API should then be ineffective.
 */
void preempt_start(bool preempt) {
    if (preempt == false) {
        return;
    }

    /* Set up handler for alarm */
    SA.sa_handler = preempt_handler;
    sigemptyset(&SA.sa_mask);
    SA.sa_flags = 0;
    sigaction(SIGALRM, &SA, NULL);
    /* Configure preempt alarm */
    alarm(1 / HZ);
}

/*
 * preempt_stop - Stop thread preemption
 *
 * Restore previous timer configuration, and previous action associated to
 * virtual alarm signals.
 */
void preempt_stop(void) {

	/* TODO Phase 4 */
}

