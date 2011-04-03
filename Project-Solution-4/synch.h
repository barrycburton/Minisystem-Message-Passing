#ifndef __SYNCH_H__
#define __SYNCH_H__

/*
 * Definitions for high-level synchronization primitives.
 *
 * You must implement the procedures and types defined in this interface.
 *
 */

typedef struct semaphore *semaphore_t;


/*
 *  Create a new semaphore object.
 *  Return NULL on failure.
 */
extern semaphore_t semaphore_create();

/*
 *  Cleanup all resources consumed by this semaphore object.
 *  Return 0 on success, -1 on failure.
 */
extern int semaphore_destroy(semaphore_t sem);
 
/*
 *  Initialize the semaphore object to represent a resource
 *  which has count units available. Keep in mind that a given
 *  semaphore object may be initialized any number of times over
 *  its lifetime, as it is used to represent different resources.
 */
extern int semaphore_initialize(semaphore_t sem, int count);


/*
 *  P (wait) on the sempahore.
 *  This function is not returned from until the semaphore
 *  is successfully aquired for the calling thread.
 */
extern void semaphore_P(semaphore_t sem);

/*
 *	V (signal) on the sempahore.
 *  This function is not returned from until the semaphore
 *  has one unit successfully released.
 */
extern void semaphore_V(semaphore_t sem);


#endif __SYNCH_H__
