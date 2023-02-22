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
#define ONE_SECOND_MICRO_SECONDS 1000000

struct itimerval timer;
struct itimerval prev_timer;
struct sigaction sa;
sigset_t ss;

int PREEMPT_BLOCKERS = 0;

// Signal handler for timer
void preempt_handler() {
    uthread_yield();
}

void preempt_disable(void) {
    // Block timer alarm
    if (PREEMPT_BLOCKERS == 0) {
        sigprocmask(SIG_BLOCK, &ss, NULL);
    }
    ++PREEMPT_BLOCKERS;
}

void preempt_enable(void) {
    // Unblock timer alarm - assumes preemption disabled, UB otherwise
    if (--PREEMPT_BLOCKERS == 0) {
        sigprocmask(SIG_UNBLOCK, &ss, NULL);
    }
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

    // SIGVTALRM signal handler
    sa.sa_handler = preempt_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGVTALRM, &sa, NULL);

    // Sig set (For signal blocking)
    sigemptyset(&ss);
    sigaddset(&ss, SIGVTALRM);

    // Timer lifetime
    int frequency = ONE_SECOND_MICRO_SECONDS / HZ; // HZ to milliseconds
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = frequency;
    // Timer interval
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = frequency;

    // Get previous timer settings
    getitimer(ITIMER_VIRTUAL, &prev_timer);

    // Set a new timer
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
        // ERROR: Failed to initialize timer
        return;
    }
}

/*
 * preempt_stop - Stop thread preemption
 *
 * Restore previous timer configuration, and previous action associated to
 * virtual alarm signals.
 */
void preempt_stop(void) {
    // Revert signal handler to default signal
    sa.sa_handler = SIG_DFL;
    sigaction(SIGVTALRM, &sa, NULL);
    // Revert timer to previous config
    setitimer(ITIMER_VIRTUAL, &prev_timer, NULL);
}