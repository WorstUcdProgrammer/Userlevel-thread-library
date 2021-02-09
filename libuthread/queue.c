#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

struct queue {
	node_t head;
	node_t tail;
	int length;	
};

struct node {
	void *data;
	node_t next;
};

node_t node_create(void)
{
	node_t new_node;
	new_node = (node_t) malloc(sizeof(struct node));
	new_node->next = NULL;
	return new_node;
}

queue_t queue_create(void)
{
	// initiate a new pointer to an empty queue
	queue_t new_queue;
	new_queue = (queue_t) malloc(sizeof(struct queue));
	new_queue->length = 0;
	// if allocating space fails, the new_queue will be a NULL pointer
	return new_queue;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->length != 0) {
		return -1;
	} else {
		free(queue);
		return 0;
	}
}

int queue_enqueue(queue_t queue, void *data)
{
	// if @queue or @data are NULL
	if (queue == NULL || data == NULL) {
		return -1;
	} else if (queue_length(queue) == 0) {
		node_t new_node = node_create();
		new_node->data = data;
		queue->head = new_node;
		queue->tail = new_node;
		queue->length++;
	} else {
		node_t new_node = node_create();
		new_node->data = data;
		queue->tail->next = new_node;
		queue->tail = new_node;
		queue->length++;
	}
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || data == NULL || queue_length(queue) == 0) {
		return -1;
	} else {
		node_t second_node = queue->head->next;
		*data = queue->head->data;
		free(queue->head);
		queue->head = second_node;
		queue->length--;
		return 0;
	}
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL) {
		return -1;
	} else {
		if (queue->head == data) {
			node_t second_node = queue->head->next;
			free(queue->head);
			queue->head = second_node;
			queue->length--;
			// if want to delete data that is both head and tail
			if (!queue->length) {
				queue->tail = NULL;
			}
			return 0;
		} else if (queue->tail == data) {
			node_t current_node = queue->head;
			while (current_node->next->next != NULL) {
				current_node = current_node->next;
			}
			free(current_node->next);
			queue->tail = current_node;
			return 0;
		} else {
			node_t current_node = queue->head;
			while (current_node->next != NULL) {
				if (current_node->next->data == data) {
					node_t next_next_node = current_node->next->next;
					free(current_node->next);
					current_node->next = next_next_node;
					queue->length--;
					return 0;
				}
				current_node = current_node->next;
			}
			return -1;
		}
	}
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if (queue == NULL || func == NULL) {
		return -1;
	} else {
		node_t current_node = queue->head;
		while (current_node != NULL) {
			node_t next_node = current_node->next;
			int func_return = (*func)(queue, current_node->data, arg);
			if (func_return == 1) {
				if (data != NULL) {
					*data = current_node->data;
				}
				return 0;
			}
			current_node = next_node;
		}
	}
	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL) {
		return -1;
	} else {
		return queue->length;
	}
}

