#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

typedef struct queue* queue_t;

int uthread_counter = 0;
queue_t queue;
queue_t zombie_queue;

enum uthread_states{
	WAITING,
	RUNNING,
	ZOMBIE
};

struct tcb {
	uthread_t TID;
	uthread_ctx_t *context;
	void *top_of_stack;
	enum uthread_states state;
	int retval;
};

typedef struct tcb* tcb_t;

tcb_t current_thread;

int uthread_start(int preempt)
{
	if (preempt) {
		return 0;
	}
	queue = queue_create();
	zombie_queue = queue_create();
	tcb_t new_tcb = (tcb_t) malloc(sizeof(struct tcb));
	if (new_tcb == NULL) {
		return -1;
	}
	new_tcb->TID = (uthread_t) 0;
	new_tcb->context = (uthread_ctx_t*) malloc(sizeof(uthread_ctx_t));
	new_tcb->state = RUNNING;
	current_thread = new_tcb;
	return 0;
}

int uthread_stop(void)
{
	if (queue_length(queue) > 0 || queue_length(zombie_queue) > 0) {
		return -1;
	} else {
		free(current_thread);
		free(queue);
		free(zombie_queue);
		return 0;
	}
}

int uthread_create(uthread_func_t func)
{
	if (uthread_counter >= USHRT_MAX) {
		return -1;
	}
	tcb_t new_tcb = (tcb_t) malloc(sizeof(struct tcb));
	// memory allocation will be handled by function
	new_tcb->top_of_stack = uthread_ctx_alloc_stack();
	if (new_tcb->top_of_stack == NULL) {
		return -1;
	}
	// allocate space for the context
	new_tcb->context = (uthread_ctx_t*) malloc(sizeof(uthread_ctx_t));
	if (uthread_ctx_init(new_tcb->context, new_tcb->top_of_stack, func)) {
		return -1;
	}
	uthread_counter++;
	new_tcb->TID = (uthread_t) uthread_counter;
	new_tcb->state = WAITING;
	queue_enqueue(queue, new_tcb);
	return (int) new_tcb->TID;
}

void uthread_yield(void)
{
	current_thread->state = WAITING;
	tcb_t data;
	tcb_t previous;
	queue_dequeue(queue, (void **)&data);
	queue_enqueue(queue, current_thread);
	data->state = RUNNING;
	previous = current_thread;
	current_thread = data;
	uthread_ctx_switch(previous->context, current_thread->context);
}

uthread_t uthread_self(void)
{
	return current_thread->TID;
}

void uthread_exit(int retval)
{
	current_thread->state = ZOMBIE;
	current_thread->retval = retval;
	queue_enqueue(zombie_queue, current_thread);
	if (queue_length(queue) != 0) {
		tcb_t data;
		tcb_t previous;
		queue_dequeue(queue, (void **)&data);
		data->state = RUNNING;
		previous = current_thread;
		current_thread = data;
		uthread_ctx_switch(previous->context, current_thread->context);
	}
}

int uthread_join(uthread_t tid, int *retval)
{
	while (1) {
		if (queue_length(queue) == 0) {
			if (retval == NULL || tid == 0) {
				break;
				return -1;
			}
			break;
			return -1;
		} else {
			uthread_yield();
		}
	}
	return 0;
}

