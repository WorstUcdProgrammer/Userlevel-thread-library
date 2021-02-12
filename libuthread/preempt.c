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

struct sigaction old_handler;
struct itimerval old_timer;
sigset_t ss;

void handler(int signum)
{
	(void) signum;
	uthread_yield();
}

void preempt_start(void)
{
	struct sigaction preempt;
	struct itimerval timer;
	/* 
	 * The usage of setitimer is learned from
	 * https://www.informit.com/articles/article.aspx?p=23618&seqNum=14
	 */
	preempt.sa_handler = handler;
	sigaction(SIGVTALRM, &preempt, &old_handler);
	timer.it_interval.tv_usec = 10000;
	timer.it_value.tv_usec = 10000;
	setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
}

void preempt_stop(void)
{
	/* restore previous */
	sigaction(SIGVTALRM, &old_handler, NULL);
	setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
}

void preempt_enable(void)
{
	/* This part of code is from 03.syscalls.pdf page 41 */
	sigprocmask(SIG_UNBLOCK, &ss, NULL);
}

void preempt_disable(void)
{
	/* This part of code is from 03.syscalls.pdf page 41 */
	sigprocmask(SIG_BLOCK, &ss, NULL);
}