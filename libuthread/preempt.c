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

bool use_preempt;
struct sigaction sig_action;
long int initial_interval = 0;
long int preempt_interval = 0;
long int current_interval = 0;

// Initialize the preempt interrupt timer
// - When process time expires, SIGVTALRM signal is generated
unsigned int preempt_set_timer(long int seconds) {
    struct itimerval prev;
    struct itimerval curr;

    curr.it_interval.tv_usec = 0;
    curr.it_interval.tv_sec = 0;
    curr.it_value.tv_usec = 0;
    curr.it_value.tv_sec = seconds;
    int retval = setitimer(ITIMER_VIRTUAL, &curr, &prev);
    // Get processes original interval value
    if (initial_interval == 0) {
        initial_interval = prev.it_value.tv_sec;
    }

    if (retval < 0) {
        return 0;
    }

    return prev.it_value.tv_sec;
}


void preempt_timer_handler(int signum) {
    uthread_yield();
}

/*
 * preempt_disable - Disable preemption
 */
void preempt_disable(void) {
    use_preempt = false;
	/* TODO Phase 4 */
}

/*
 * preempt_enable - Enable preemption
 */
void preempt_enable(void) {
    use_preempt = true;
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
    if (preempt == false || use_preempt == false) {
        return;
    }

    /* Set up handler for alarm */
    sig_action.sa_handler = preempt_timer_handler;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;
    sigaction(SIGVTALRM, &sig_action, NULL);
    /* Configure preempt alarm */
    long int seconds = 1 / HZ;
    preempt_set_timer(seconds);
}

/*
 * preempt_stop - Stop thread preemption
 *
 * Restore previous timer configuration, and previous action associated to
 * virtual alarm signals.
 */
void preempt_stop(void) {
    if (use_preempt == false) {
        return;
    }
	/* TODO Phase 4 */
}

