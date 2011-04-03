/*
 *	API Definitions for our minithread library
 */

#ifndef __MINITHREAD_H__
#define __MINITHREAD_H__


/* handle for a thread */
typedef struct minithread *minithread_t;

/* generic function argument */
typedef int *arg_t;

/* generic function pointer */
typedef int (*proc_t)(arg_t);

/* generic lock type */
typedef int tas_lock_t;


/*
 *	Create and schedule a new thread of control. When the
 *  scheduler chooses the thread, it will start executing
 *  the function refered to by proc, with it being called
 *  with arg as the single argument.
 */	
extern minithread_t minithread_fork(proc_t proc, arg_t arg);


/*
 *	Create a new thread, but do not schedule it for execution.
 *  After start is called on the thread it will be added to
 *  the scheduler. After this, when the scheduler chooses the
 *  the thread, it will start executing the function refered
 *  to by proc, with it being called with arg as the single
 *  argument.
 */
extern minithread_t minithread_create(proc_t proc, arg_t arg);


/*
 *	Return handle (minithread_t) of calling thread.
 */
extern minithread_t minithread_self();


/*
 *  Return thread identifier of calling thread.
 */
extern int minithread_id();


/*
 *	Block the calling thread. This yields the
 *  processor but does not place the thread
 *  back into the scheduler. Note that the thread
 *  still exists and if another thread calls
 *  minithread_start, it will be placed back
 *  in the scheduler and once chosen will
 *  resume executing after this call.
 */
extern int minithread_stop();


/*
 *	Make thread runnable.
 */
extern int minithread_start(minithread_t thread);


/*
 *	Forces the caller to relinquish the processor and be put to the end of
 *	the ready queue.  Allows another thread to run.
 */
extern int minithread_yield();


/*
 *	Initialize the system to run the first minithread at
 *	mainproc(mainarg).  This procedure should be called from your
 *	main program with the callback procedure and argument specified
 *	as arguments.
 */
extern int minithread_system_initialize(proc_t mainproc, arg_t mainarg);


/*
 *	Atomically release the specified test-and-set lock and
 *	block the calling thread. This is a convenience function that
 *  does not add much (if any) power, but makes for more readable
 *  code and simplifies the possible system states, making it
 *  easier to reason about application correctness.
 */
extern int minithread_unlock_and_stop(tas_lock_t *lock);


/*
 *  Atomically set an alarm and then block the calling thread.
 *  The alarm will be for delay milliseconds and upon firing
 *  will simply unblock this thread. This is a convenience function that
 *  does not add much (if any) power, but makes for more readable
 *  code and simplifies the possible system states, making it
 *  easier to reason about application correctness.
 */
extern int minithread_sleep_with_timeout(int delay);


#endif __MINITHREAD_H__
