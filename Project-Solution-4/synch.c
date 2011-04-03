
#include "machineprimitives.h"
#include "defs.h"

#include "minithread.h"
#include "synch.h"
#include "queue.h"



/*
 *	You must implement the procedures and types defined in this interface.
 */


/*
 * Semaphores.
 */

struct semaphore {
	int count;
	tas_lock_t lock;
	queue_t thread_queue;
};

/*
 * semaphore_t semaphore_create()
 *	Allocate a new semaphore.
 */
semaphore_t semaphore_create() {
	semaphore_t sem = (semaphore_t)malloc(sizeof(struct semaphore));
	if (sem == NULL)
		return NULL;
	sem->thread_queue = queue_new();
	if (sem->thread_queue == NULL) {
		free(sem);
		return NULL;
	}
	sem->count = 0;
	sem->lock = 0;
	return sem;
}

/*
 * semaphore_destroy(semaphore_t sem);
 *	Deallocate a semaphore.
 */
int semaphore_destroy(semaphore_t sem) {
	queue_free(sem->thread_queue);
	free(sem);
	return 0;
}

/*
 * semaphore_initialize(semaphore_t sem, int cnt)
 *	initialize the semaphore data structure pointed at by
 *	sem with an initial value cnt.
 */
int semaphore_initialize(semaphore_t sem, int count) {
	sem->count = count;
	return 0;
}

//spin on a lock until it becomes available
void semaphore_spinlock(tas_lock_t *lock) {
	while (atomic_test_and_set(lock))
		minithread_yield();
}

/*
 * semaphore_P(semaphore_t sem)
 *	P on the sempahore.
 */
void semaphore_P(semaphore_t sem) {
	//Loop until we succeed
	semaphore_spinlock(&(sem->lock));
	while (sem->count == 0) {
		queue_append(sem->thread_queue, minithread_self());
		atomic_clear(&(sem->lock));
		minithread_stop();
		semaphore_spinlock(&(sem->lock));
	}

	//Got the semaphore. Decrement and break
	sem->count--;
	atomic_clear(&(sem->lock));
}


/*
 * semaphore_V(semaphore_t sem)
 *	V on the sempahore.
 * If a thread is waiting to P the semaphore, 
 * wake it up.
 */
void semaphore_V(semaphore_t sem) {
	minithread_t waiting_thread;

	semaphore_spinlock(&(sem->lock));
	sem->count++;
	if ( queue_length(sem->thread_queue) ) {
		queue_dequeue(sem->thread_queue, &waiting_thread);
		minithread_start(waiting_thread);
	}
	atomic_clear(&(sem->lock));
}
