/*
 * priority_queue.c - implements our priority_queue ADT
 */

#include "defs.h"

/*
 * TYPE DEFINITIONS
 */

/*
 * any_t is a void pointer. This allows you to put arbitrary structures in
 * the priority queue.
 */
typedef void *any_t;


/*
 * PFany is a pointer to a function that takes two any_t arguments
 * and returns an integer.
 */
typedef int (*PFany)(any_t, any_t);


/*
 * priority_queue holds all the state associated
 * with an instance of our ADT
 */
struct priority_queue {
	struct prio_node *first;
	int count;
};
typedef struct priority_queue *priority_queue_t;

struct prio_node {
	any_t item;
	int prio;
	struct prio_node *next;
};

/*
 * priority_queue_order is an enumeration which is used to specify
 * whether larger values are considered higher priority or vice
 * versa.
 */
enum priority_queue_order {
	PQ_PRIORITY_ASCEND, /* 0 is highest priority - dequeue outputs items of priority of ascending integer values */
	PQ_PRIORITY_DESCEND /* 0 is lowest priority - dequeue outputs items of priority of descending integer values */
};
typedef enum priority_queue_order priority_queue_order_t;



/*
 * FUNCTION DECLARATIONS
 */

/*
 * Create a new priority queue data structure which should be initialized
 * and will contain no entries. order tells how integer values will be
 * mapped to priority in this priority queue.
 * On success return a pointer to the new priority queue.
 * On failure return NULL.
 */
priority_queue_t priority_queue_new(priority_queue_order_t order) {
	priority_queue_t obj = malloc(sizeof(struct priority_queue));
	obj->first = NULL;
	obj->count = 0;
	return obj;
}


/*
 * Enqueue an item with specified priority to a priority queue
 * (all specifed as parameters).
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of the
 * priority queue should left as it was before this call.
 */
int priority_queue_enqueue(priority_queue_t obj, int priority, any_t item) {
	if ( obj ) {
		struct prio_node *new_node = malloc(sizeof(struct prio_node));
		new_node->item = item;
		new_node->prio = priority;
		new_node->next = NULL;
		if ( obj->count == 0 ) {
			obj->first = new_node;
		} else {
			struct prio_node *temp = obj->first;
			struct prio_node *prev = NULL;
			while ( temp && temp->prio < priority ) {
				prev = temp;
				temp = temp->next;
			}
			if ( prev == NULL ) {
				new_node->next = obj->first;
				obj->first = new_node;
			} else {
				prev->next = new_node;
				new_node->next = temp;
			}
		}
		obj->count++;
		return 0;
	}
	return -1;
}

/*
 * Dequeue and return the highest priority item in the priority queue.
 * Return 0 (success) and the dequeued item if the priority queue is
 * valid and nonempty.
 * Return -1 (failure) and NULL if the priority queue is invalid or
 * empty.
 * The second parameter is a pointer to any_t pointer (called
 * a double pointer) which allows us to pass in an any_t
 * by reference. Internally, this parameter will be dereferenced
 * and the 2nd return value assigned to it, which affects the
 * variable in the caller's scope, so even though this is not true
 * returned value in the functional sense, we will still use
 * the term return value.
 */
int priority_queue_dequeue(priority_queue_t obj, any_t* item_p) {
	if ( obj && obj->count > 0 ) {
		struct prio_node *temp;
		*item_p = obj->first->item;
		temp = obj->first;
		obj->first = obj->first->next;
		free(temp);
		obj->count--;
		return 0;
	}
	*item_p = NULL;
	return -1;
}


/*
 * Return the highest priority item in the priority queue.
 * Just like dequeue, with the exception that the item remains in the queue.
 * Return 0 (success) and the dequeued item if the priority queue is
 * valid and nonempty.
 * Return -1 (failure) and NULL if the priority queue is invalid or
 * empty.
 */
int priority_queue_peak(priority_queue_t obj, any_t* item_p) {
	if ( obj && obj->count > 0 ) {
		*item_p = obj->first->item;
		return 0;
	}
	*item_p = NULL;
	return -1;
}


/*
 * If any parameters are invalid or the priority queue is empty,
 * return -1 (failure).
 * Otherwise, iterate through the priority queue, calling the iter_func function
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
int priority_queue_iterate(priority_queue_t obj, PFany iter_func, any_t item) {
	return 0;
}


/* 
 * If the priority queue is valid, free all memory it uses
 * and return 0 (success).
 * Otherwise, return -1 (failure).
 */
int priority_queue_free(priority_queue_t obj) {
	if ( obj ) {
		struct prio_node *temp;
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
 * If the priority queue is valid, return the number of items
 * in the priority queue (a valid priority queue may be empty).
 * Otherwise, return -1 (failure).
 */
int priority_queue_length(priority_queue_t obj) {
	return obj->count;
}


/* 
 * If the priority queue is valid and the item is in the priority queue,
 * remove the item from the priority queue, and return 0 (success).
 * Otherwise, return -1 (failure)
 */
int priority_queue_delete(priority_queue_t obj, any_t item) {
	if ( obj && obj->count != 0 ) {
		struct prio_node *temp = obj->first;
		struct prio_node *prev = NULL;
		while ( temp && temp->item != item ) {
			prev = temp;
			temp = temp->next;
		}
		if ( temp->item == item ) {
			if ( prev == NULL ) {
				obj->first = temp->next;
			} else {
				prev->next = temp->next;
			}
			free(temp);
			obj->count--;
			return 0;
		}
	}
	return -1;
}
