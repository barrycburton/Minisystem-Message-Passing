/*
 * multilevel_queue.c - implements our multilevel queue ADT
 */

#include "defs.h"
#include "queue.h"

/*
 * TYPE DEFINITIONS
 */

/*
 * any_t is a void pointer. This allows you to put arbitrary structures in
 * the multilevel queue.
 */
typedef void *any_t;


/*
 * PFany is a pointer to a function that takes two any_t arguments
 * and returns an integer.
 */
typedef int (*PFany)(any_t, any_t);


/*
 * multilevel_queue holds all of the state associated
 * with an instance of our ADT
 */
struct multilevel_queue {
	queue_t *levels;
	int num;
	int size;
};
typedef struct multilevel_queue *multilevel_queue_t;


/*
 * multilevel_queue_order is an enumeration which is used to specify
 * whether larger values are considered to be higher levels or vice
 * versa.
 */
enum multilevel_queue_order {
	MQ_LEVEL_ASCEND, /* 0 is highest level - dequeue outputs items from levels of ascending integer values */
	MQ_LEVEL_DESCEND /* 0 is lowest level - dequeue outputs items from levels of descending integer values */
};
typedef enum multilevel_queue_order multilevel_queue_order_t;



/*
 * FUNCTION DECLARATIONS
 */

/*
 * Create a new multilevel queue data structure which should be initialized
 * and will contain no entries. num_levels tells how many levels the queue 
 * will have order tells how integer values will be mapped to levels in
 * this multilevel queue.
 * On success return a pointer to the new multilevel_queue.
 * On failure return NULL.
 */
multilevel_queue_t multilevel_queue_new(int num_levels, multilevel_queue_order_t order) {
	int i;
	multilevel_queue_t ret = malloc(sizeof(struct multilevel_queue));
	ret->num = num_levels;
	ret->size = 0;
	ret->levels = malloc(sizeof(queue_t)*num_levels);
	for(i=0;i<num_levels;i++) {
		ret->levels[i] = queue_new();
	}
	return ret;
}


/*
 * Enqueue an item to the specified level in a multilevel queue
 * (all specifed as parameters).
 * Within a level, FIFO order should be followed.
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of the multilevel queue
 * should left as it was before this call.
 */
int multilevel_queue_enqueue(multilevel_queue_t obj, int level, any_t item) {
	queue_append(obj->levels[level], item);
	obj->size++;
	return 0;
}


/*
 * Dequeue and return the first item from the highest level that is not higher
 * than the specified level. 
 * Return 0 (success) and the dequeued item if the multilevel queue is
 * valid and nonempty.
 * Return -1 (failure) and NULL if the multilevel queue is invalid or
 * empty.
 * The second parameter is a pointer to any_t pointer (called
 * a double pointer) which allows us to pass in an any_t
 * by reference. Internally, this parameter will be dereferenced
 * and the 2nd return value assigned to it, which affects the
 * variable in the caller's scope, so even though this is not true
 * returned value in the functional sense, we will still use
 * the term return value.
 */
int multilevel_queue_dequeue(multilevel_queue_t obj, int level, any_t* item_p) {
	any_t item = NULL;
	int ret = queue_dequeue(obj->levels[level], &item);
	while ( level < (obj->num - 1)&& -1 == ret ) {
		level++;
		ret = queue_dequeue(obj->levels[level], &item);
	}	
	if ( ret != -1 ) {
		*item_p = item;
	}
	obj->size--;
	return ret;
}


/*
 * Return the first item from the highest level that is not higher
 * than the specified level - just like dequeue, with the exception
 * that the item remains in the queue.
 * Return 0 (success) and the dequeued item if the multilevel queue is
 * valid and nonempty.
 * Return -1 (failure) and NULL if the multilevel queue is invalid or
 * empty.
 */
int multilevel_queue_peak(multilevel_queue_t obj, int level, any_t* item_p) {
	any_t item = NULL;
	int ret = queue_dequeue(obj->levels[level], &item);
	while ( (level < (obj->num - 1)) && -1 == ret ) {
		level++;
		ret = queue_dequeue(obj->levels[level], &item);
	}	
	if ( ret != -1 ) {
		queue_prepend(obj->levels[level], item);
		*item_p = item;
	}
	return ret;
}

/*
 * If any parameters are invalid or the multilevel queue is empty,
 * return -1 (failure).
 * Otherwise, iterate through the multilevel queue, calling the iter_func function
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
int multilevel_queue_iterate(multilevel_queue_t obj, PFany iter_func, any_t item) {
	return 0;
}


/* 
 * If the multilevel queue is valid, free all memory it uses
 * and return 0 (success).
 * Otherwise, return -1 (failure).
 */
int multilevel_queue_free(multilevel_queue_t obj) {
	int i;
	for(i=0;i<obj->num;i++) {
		queue_free(obj->levels[i]);
	}
	free(obj->levels);
	free(obj);
	return 0;
}


/*
 * If the multilevel queue is valid, return the number of items
 * in the multilevel queue (a valid multilevel queue may be empty).
 * Otherwise, return -1 (failure).
 */
int multilevel_queue_length(multilevel_queue_t obj) {
	return obj->size;
}


/* 
 * If the multilevel queue is valid and the item is in the multilevel queue,
 * remove the item from the multilevel queue, and return 0 (success).
 * Otherwise, return -1 (failure)
 */
int multilevel_queue_delete(multilevel_queue_t obj, any_t item) {
	return 0;
}

