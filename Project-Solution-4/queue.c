/*
 * queue.c - fill in your implementation
 * in this file
 */


/*
 * INCLUDES
 */
#include <stdlib.h>

#include "defs.h"

#include "queue.h"



/*
 * DATA STRUCTURE DEFINITIONS
 */
struct queue {
	struct node *first;
	struct node *last;
	int count;
};

struct node {
	any_t item;
	struct node *next;
};


/*
 * FUNCTION DEFINITIONS
 */

/*
 * Create a new queue data structure which should be initialized
 * and will contain no entries. 
 * On success return a pointer to the new queue.
 * On failure return NULL.
 */
queue_t queue_new() {
	queue_t obj = malloc(sizeof(struct queue));
	obj->first = obj->last = NULL;
	obj->count = 0;
	return obj;
}


/*
 * Prepend an any_t to a queue (both specifed as parameters).
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of queue
 * should left as it was before this call.
 */
int queue_prepend(queue_t obj, any_t item) {
	if ( obj ) {
		struct node *new_node = malloc(sizeof(struct node));
		new_node->item = item;
		new_node->next = NULL;
		if ( obj->count == 0 ) {
			obj->first = obj->last = new_node;
		} else {
			new_node->next = obj->first;
			obj->first = new_node;
		}
		obj->count++;
		return 0;
	}
	return -1;
}


/*
 * Append an any_t to a queue (both specifed as parameters).
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of queue
 * should left as it was before this call.
 */
int queue_append(queue_t obj, any_t item) {
	if ( obj ) {
		struct node *new_node = malloc(sizeof(struct node));
		new_node->item = item;
		new_node->next = NULL;
		if ( obj->count == 0 ) {
			obj->first = obj->last = new_node;
		} else {
			obj->last->next = new_node;
			obj->last = new_node;
		}
		obj->count++;
		return 0;
	}
	return -1;
}


/*
 * Dequeue and return the first any_t from the queue.
 * Return 0 (success) and the dequeued item if queue is
 * valid and nonempty.
 * Return -1 (failure) and NULL if the queue is invalid or
 * empty.
 * The second parameter is a pointer to any_t pointer (called
 * a double pointer) which allows us to pass in an any_t
 * by reference. Internally, this parameter will be dereferenced
 * and the 2nd return value assigned to it, which affects the
 * variable in the caller's scope, so even though this is not true
 * returned value in the functional sense, we will still use
 * the term return value.
 */
int queue_dequeue(queue_t obj, any_t* item_p) {
	if ( obj && obj->count > 0 ) {
		struct node *temp;
		*item_p = obj->first->item;
		temp = obj->first;
		obj->first = obj->first->next;
		if ( !obj->first ) {
			obj->last = NULL;
		}
		free(temp);
		obj->count--;
		return 0;
	}
	*item_p = NULL;
	return -1;
}


/*
 * If any parameters are invalid or the queue is empty return -1 (failure).
 * Otherwise, iterate through the queue, calling the iter_func function
 * on each item in the queue. (The current queue item is passed
 * as the first paramter to iter_func, and any_t data is passed as
 * the second paramter. If at any point iter_func returns -1,
 * then stop iterating and return -1 (failure).
 * Otherwise, return 0 (success).
 * At first glance this function will seem useless, but keep in mind
 * that data is a void pointer type and so can be a pointer to anything,
 * so values can returned out via it (which also affects the value received
 * by the function on the next iteration), or it can point to a larger structure
 * whose fields are modified by iter_func.
 */
int queue_iterate(queue_t obj, PFany iter_func, any_t data) {
	if ( obj && obj->count > 0 && iter_func ) {
		struct node *temp = obj->first;
		int ret = 0;
		while ( temp && ret == 0) {
			(*iter_func)(temp->item, data);
			temp = temp->next;
		}
		return ret;
	}
	return -1;
}


/* 
 * If the queue is valid, free all memory it uses
 * and return 0 (success).
 * Otherwise, return -1 (failure).
 */
int queue_free(queue_t obj) {
	if ( obj ) {
		struct node *temp;
		while ( obj->first ) {
			temp = obj->first;
			obj->first = obj->first->next;
			free(temp);
		}
		free(obj);
		return 0;
	}
	return -1;	
}


/*
 * If the queue is valid, return the number of items
 * in the queue (a valid queue may be empty).
 * Otherwise, return -1 (failure).
 */
int queue_length(queue_t obj) {
	if ( obj ) {
		return obj->count;
	}
	return -1;
}


/* 
 * If the queue is valid and the item is in the queue,
 * remove the item from the queue, and return 0 (success).
 * Otherwise, return -1 (failure).
 */
int queue_delete(queue_t obj, any_t item) {
	if ( obj && obj->count > 0 ) {
		struct node *temp = obj->first;
		struct node *last = NULL;
		int flag = 0;
		while ( temp ) {
			if ( temp->item == item ) {
				if ( obj->first == temp ) {
					obj->first = temp->next;
					if ( obj->first == NULL ) {
						obj->last = NULL;
					}
					free(temp);
					temp = obj->first;
				} else if ( obj->last == temp ) {
					obj->last = last;
					last->next = NULL;
					if ( obj->last == NULL ) {
						obj->last = obj->first;
					}
					free(temp);
					temp = NULL;
				} else {
					last->next = temp->next;
					free(temp);
					temp = last->next;
				}
				
				obj->count--;
				flag = 1;
			} else {
				last = temp;
				temp = temp->next;
			}
		}
		if ( flag ) {
			return 0;
		}
	}
	return -1;
}
