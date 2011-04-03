/*
 * queue.h - defines the interface of our queue ADT
 * this file may be thought of a has having most of the
 * properties of a class - it is a self contained data 
 * abstraction with included control logic for operations
 * on the data
 */

/*
 * this define is called a header guard. it prevents header
 * files from being included more than once in the same
 * compilation, as the C compiler dislikes it when
 * things are defined more than once.
 */
#ifndef __QUEUE_H__
#define __QUEUE_H__


/*
 * TYPE DEFINITIONS
 */

/*
 * any_t is a void pointer. This allows you to put arbitrary structures on
 * the queue.
 */
typedef void *any_t;


/*
 * PFany is a pointer to a function that can take two any_t arguments
 * and return an integer.
 */
typedef int (*PFany)(any_t, any_t);


/*
 * queue_t is defined to be a pointer type which points to a
 * a struct queue structure. the definition of this structure
 * need not be known
 */
typedef struct queue *queue_t;



/*
 * FUNCTION DECLARATIONS
 */

/*
 * Create a new queue data structure which should be initialized
 * and will contain no entries. 
 * On success return a pointer to the new queue.
 * On failure return NULL.
 */
extern queue_t queue_new();


/*
 * Prepend an any_t to a queue (both specifed as parameters).
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of queue
 * should left as it was before this call.
 */
extern int queue_prepend(queue_t obj, any_t item);


/*
 * Append an any_t to a queue (both specifed as parameters).
 * On success return 0 (success).
 * On failure return -1 (failure). In this case the state of queue
 * should left as it was before this call.
 */
extern int queue_append(queue_t obj, any_t item);


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
extern int queue_dequeue(queue_t obj, any_t* item_p);


/*
 * Iterate through the queue, calling the PFany function
 * on each item in the queue. (The current queue item is 
 * passed as the first paramter, and the additional
 * The additional any_t argument is passed to the function as its first
 * argument and the queue element is the second.  
 * Return 0 (success) or -1 (failure).
 */
extern int queue_iterate(queue_t obj, PFany iter_func, any_t item);


/* 
 * If the queue is valid, free all memory it uses
 * and return 0 (success).
 * Otherwise, return -1 (failure).
 */
extern int queue_free(queue_t obj);


/*
 * If the queue is valid, return the number of items
 * in the queue (a valid queue may be empty).
 * Otherwise, return -1 (failure).
 */
extern int queue_length(queue_t obj);


/* 
 * If the queue is valid and the item is in the queue,
 * remove the item from the queue, and return 0 (success).
 * Otherwise, return -1 (failure)
 */
extern int queue_delete(queue_t obj, any_t item);


#endif __QUEUE_H__
