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
queue_t all_threads;

enum uthread_states{
	WAITING,
	RUNNING,
	ZOMBIE,
	BLOCKED,
	DELETED
};

struct tcb {
	uthread_t TID;
	uthread_ctx_t *context;
	void *top_of_stack;
	enum uthread_states state;
	int retval;
	struct tcb *blocked;
};

typedef struct tcb* tcb_t;

tcb_t current_thread;

void free_resource(tcb_t dead_tcb)
{
	free(dead_tcb->context);
	uthread_ctx_destroy_stack(dead_tcb->top_of_stack);
	if (dead_tcb->blocked != NULL) {
		dead_tcb->blocked->state = WAITING;
		queue_enqueue(queue, dead_tcb->blocked);
		dead_tcb->blocked = NULL;
	}
	//queue_delete(all_threads, dead_tcb);
	dead_tcb->state = DELETED;
}

int uthread_start(int preempt)
{
	if (preempt) {
		return 0;
	}
	queue = queue_create();
	all_threads = queue_create();
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

static int delete_all(queue_t q, void *data, void *arg)
{
    (void)q;
	if (data == NULL || arg == NULL) {
		free(data);
	}
    return 0;
}

int uthread_stop(void)
{
	if (queue_length(queue) > 0 || queue_length(all_threads) > 1) {
		return -1;
	} else {
		free(queue);
		free(all_threads);
		queue_iterate(all_threads, delete_all, NULL, NULL);
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
	new_tcb->blocked = NULL;
	queue_enqueue(queue, new_tcb);
	queue_enqueue(all_threads, new_tcb);
	return (int) new_tcb->TID;
}

void uthread_yield(void)
{
	// if there are no more other threads available to run
	// continue to run the current thread
	// thus, yield doesn't make any changes
	if (queue_length(queue) > 0) {
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
}

uthread_t uthread_self(void)
{
	return current_thread->TID;
}

void uthread_exit(int retval)
{
	current_thread->state = ZOMBIE;
	current_thread->retval = retval;
	if (queue_length(all_threads) > 1) {
		if (queue_length(queue) != 0) {
			tcb_t data;
			tcb_t previous;
			queue_dequeue(queue, (void **)&data);
			data->state = RUNNING;
			previous = current_thread;
			current_thread = data;
			if (previous->blocked != NULL) {
				free_resource(previous);
			}
			uthread_ctx_switch(previous->context, current_thread->context);
		} else {
			tcb_t previous = current_thread;
			tcb_t new_tcb = current_thread->blocked;
			current_thread->blocked = NULL;
			current_thread = new_tcb;
			free_resource(previous);
			uthread_ctx_switch(previous->context, current_thread->context);
		}
	}
}

static int find_thread(queue_t q, void *data, void* arg)
{
	uthread_t a = ((tcb_t) data)->TID;
	int *match = (int*) arg;
	(void)q;
 
	if ((int) a == *match)
		return 1;
	
	return 0;
}

int uthread_join(uthread_t tid, int *retval)
{
	tcb_t ptr = NULL;
	queue_iterate(all_threads, find_thread, (void*)&tid, (void**)&ptr);
	if (ptr == NULL || tid == (uthread_t) 0) {
		return -1;
	} else {
		if (ptr->blocked != NULL || ptr->state == DELETED) {
			return -1;
		} else if (ptr->state == ZOMBIE) {
			free_resource(ptr);
			if (retval != NULL) {
				*retval = ptr->retval;
			}
		} else {
			tcb_t previous;
			tcb_t data;
			queue_dequeue(queue, (void **)&data);
			data->state = RUNNING;
			previous = current_thread;
			ptr->blocked = current_thread;
			current_thread->state = BLOCKED;
			current_thread = data;
			uthread_ctx_switch(previous->context, current_thread->context);
		}
		return 0;
	}
}