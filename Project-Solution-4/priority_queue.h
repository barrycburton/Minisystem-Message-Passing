/*
 * priority_queue.h - defines the interface of our priority_queue ADT
 * this file may be thought of a has having most of the
 * properties of a class - it is a self contained data 
 * abstraction with included control logic for operations
 * on the data
 */

#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__



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
 * priority_queue_t is defined to be a pointer type which points to a
 * a struct priority_queue structure. the definition of this structure
 * need not be known here
 */
typedef struct priority_queue *priority_queue_t;


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
extern priority_queue_t priority_queue_new(priority_queue_order_t order);


/*
 * Enqueue an item with specified priority to a priority queue
 * (all specifed as parameters).
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of the
 * priority queue should left as it was before this call.
 */
extern int priority_queue_enqueue(priority_queue_t obj, int priority, any_t item);


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
extern int priority_queue_dequeue(priority_queue_t obj, any_t* item_p);


/*
 * Return the highest priority item in the priority queue.
 * Just like dequeue, with the exception that the item remains in the queue.
 * Return 0 (success) and the dequeued item if the priority queue is
 * valid and nonempty.
 * Return -1 (failure) and NULL if the priority queue is invalid or
 * empty.
 */
extern int priority_queue_peak(priority_queue_t obj, any_t* item_p);


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
extern int priority_queue_iterate(priority_queue_t obj, PFany iter_func, any_t item);


/* 
 * If the priority queue is valid, free all memory it uses
 * and return 0 (success).
 * Otherwise, return -1 (failure).
 */
extern int priority_queue_free(priority_queue_t obj);


/*
 * If the priority queue is valid, return the number of items
 * in the priority queue (a valid priority queue may be empty).
 * Otherwise, return -1 (failure).
 */
extern int priority_queue_length(priority_queue_t obj);


/* 
 * If the priority queue is valid and the item is in the priority queue,
 * remove the item from the priority queue, and return 0 (success).
 * Otherwise, return -1 (failure)
 */
extern int priority_queue_delete(priority_queue_t obj, any_t item);


#endif __PRIORITY_QUEUE_H__
