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
#define ONE_NANO_SECOND 1000000000

bool use_preempt;
struct sigaction sa;
struct itimerval timer;

void preempt_handler(int signum) {
    if (signum != SIGVTALRM) {
        return;
    }
    printf("signaled\n");
    uthread_yield();
}

/*
 * preempt_disable - Disable preemption
 */
void preempt_disable(void) {
    use_preempt = false;
}

/*
 * preempt_enable - Enable preemption
 */
void preempt_enable(void) {
    use_preempt = true;
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

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    sa.sa_handler = preempt_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGVTALRM, &sa, NULL);

    // Timer lifetime
    long interval = ONE_NANO_SECOND / HZ;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = interval;
    // Timer interval
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = interval;

    if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
        printf("wsl sucks\n");
    }
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

    // Revert signal handler to default signal
    sa.sa_handler = SIG_DFL;
    sigaction(SIGVTALRM, &sa, NULL);
    // Revert timer to previous config
}