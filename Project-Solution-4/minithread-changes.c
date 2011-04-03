/*
 * minithread.c:
 *	This file provides a few function headers for the procedures that
 *	you are required to implement for the minithread assignment.
 *
 *	EXCEPT WHERE NOTED YOUR IMPLEMENTATION MUST CONFORM TO THE
 *	NAMING AND TYPING OF THESE PROCEDURES.
 *
 */

#include "defs.h"
#include "machineprimitives.h"
#include "interrupts.h"

#include "queue.h"
#include "synch.h"
#include "multilevel_queue.h"
#include "alarm_private.h"
#include "minithread.h"


// PERIOD is defined in interrupts.h and this is the
// the number of milliseconds between clock interrupts

// ticks is global variable which is declared in interrupts.h
// and which tells how many PERIODs have elapsed since the
// clock interrupts were started


// define the length of a short in terms of periods
#define SHORT_QUANTA (1)

// define the length of a long quanta in terms of short quanta
#define LONG_QUANTA_SHORTS (4)

// define how much time (ms) is in a short quanta
#define SHORT_QUANTA_MS (SHORT_QUANTA * PERIOD)

// define how many periods are in a long quanta
#define LONG_QUANTA (LONG_QUANTA_SHORTS * SHORT_QUANTA)

// define how much time (ms) is in a long quanta
#define LONG_QUANTA_MS (LONG_QUANTA * PERIOD)

// define priority of short running thread
#define PRIORITY_SHORT (0)

// define priority of long running thread
#define PRIORITY_LONG (1)

// define age (in periods) at which long running thread is promoted to short
#define PROMOTE_AGE (8)



/*
 * A minithread should be defined either in this file or in a private
 * header file.  Minithreads have a stack pointer with to make procedure
 * calls, a stackbase which points to the bottom of the procedure
 * call stack, the ability to be enqueueed and dequeued, and any other state
 * that you feel they must have.
 */
struct minithread {
	int id;
	stack_pointer_t sp;
	stack_pointer_t sb;
	int priority;
	long age;
};

//The currently executing minithread
minithread_t current = NULL;
minithread_t idle = NULL;

//The ready queue of threads
multilevel_queue_t ready_queue;

// the stopped queue
queue_t stop_queue;

//A queue of threads which have died, and need to be freed
queue_t dead_queue;

//The id of the last created thread
int last_id;

// the ticks at which the current quanta ends
long current_quanta_end;


/* Internal Function Declarations */
/* (definitions at bottom of file) */
int minithread_cleanup(arg_t arg);
int minithread_idle(void);
int minithread_schedule(void);
int minithread_system_cleanup(void);
int minithread_age(multilevel_queue_t queue);
void minithread_clock_handler(void *arg);
void minithread_awaken_callback(arg_t arg);




/* minithread functions */

//Create a new thread and call minithread_start on it
minithread_t
minithread_fork(proc_t proc, arg_t arg) {
	minithread_t new_thread = minithread_create(proc, arg);
	if (new_thread == NULL)
		return NULL;
	minithread_start(new_thread);
	return new_thread;
}

//Create a new threads
minithread_t
minithread_create(proc_t proc, arg_t arg) {
	minithread_t new_thread = (minithread_t)malloc(sizeof(struct minithread));
	if (new_thread == NULL)
		return NULL;
	new_thread->id=++last_id;
	new_thread->priority = PRIORITY_SHORT;
	minithread_allocate_stack(&(new_thread->sb), &(new_thread->sp));
	minithread_initialize_stack(
			&(new_thread->sp),
			proc,
			arg,
			minithread_cleanup,
			NULL
		);
	return new_thread;
}

//Returns current, the currently executing thread
minithread_t minithread_self() {
	return current;
}

//Return the id of the current thread
int minithread_id() {
	return current->id;
}

//Block the current thread, switching execution
//to the next thread in the ready queue
int minithread_stop() {
	current->priority = PRIORITY_SHORT;
	queue_append(stop_queue, current);
	minithread_schedule();
	return 0;
}

//Place the argument thread on the ready queue
int minithread_start(minithread_t t) {
	queue_delete(stop_queue, t);
	t->age = ticks;
	multilevel_queue_enqueue(ready_queue, current->priority, t);
	return 0;
}

//Yield execution to the next thread

int minithread_yield() {
	current->priority = PRIORITY_SHORT;
	current->age = ticks;
	multilevel_queue_enqueue(ready_queue, current->priority, current);
	minithread_schedule();
	return 0;
}

//Free memory for the argument thread.
int minithread_free(minithread_t thread) {
	if ( thread->sb ) {
		minithread_free_stack(thread->sb);
	}
	free(thread);
	return 0;
}


/*
 *	Atomically release the specified test-and-set lock and
 *	block the calling thread. This is a convenience function that
 *  does not add much (if any) power, but makes for more readable
 *  code and simplifies the possible system states, making it
 *  easier to reason about application correctness.
 */
int minithread_unlock_and_stop(tas_lock_t *lock) {
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	atomic_clear(lock);
	minithread_stop();
	set_interrupt_level(old_int);
	return 0;
}


/*
 *  Atomically set an alarm and then block the calling thread.
 *  The alarm will be for delay milliseconds and upon firing
 *  will simply unblock this thread. This is a convenience function that
 *  does not add much (if any) power, but makes for more readable
 *  code and simplifies the possible system states, making it
 *  easier to reason about application correctness.
 */
int minithread_sleep_with_timeout(int delay) {
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	alarm_register(delay, minithread_awaken_callback, (arg_t)current, NULL);
	set_interrupt_level(old_int);
	return 0;
}


/*
 * Initialization.
 *
 * 	minithread_system_initialize:
 *	 This procedure should be called from your C main procedure
 *	 to turn a single threaded UNIX process into a multithreaded
 *	 program.
 *
 *	 Initialize any private data structures.
 * 	 Create the idle thread.
 *       Fork the thread which should call mainproc(mainarg)
 * 	 Start scheduling.
 *
 */
int minithread_system_initialize(proc_t mainproc, arg_t mainarg) {
	dbgprintf("Initializing minithread system...\n");
	ready_queue = multilevel_queue_new(2, 0);
	stop_queue = queue_new();
	dead_queue = queue_new();

	if (!ready_queue || !stop_queue || !dead_queue ) {
		return 0;
	}

	last_id = 0;
	current_quanta_end = 0;

	current = idle = minithread_create(NULL, NULL);
	minithread_fork(mainproc, mainarg);

	// initialize alarm subsystem
	alarm_system_initialize();

	minithread_clock_init(minithread_clock_handler);
	// interrupts currently disabled

	// schedule will call switch, which will enable interrupts
	minithread_schedule();

	// begin idle thread body
	minithread_idle();

	// system is shutting down now
	// stop clock interrupts
//	minithread_clock_stop();

	// cleanup alarm system
	alarm_system_cleanup();

	// cleanup thread system
	minithread_system_cleanup();

	return 0;
}

int minithread_system_cleanup(void) {
	multilevel_queue_free(ready_queue);
	queue_free(stop_queue);
	queue_free(dead_queue);
	minithread_free(idle);
	dbgprintf("minithread system cleaned up and shut down.\n");
	return 0;
}


/* 
 * Internal Functions (only used in c file, so input does not
 * have to be checked thoroughly, but function prototypes MUST NOT
 * be listed in header file.
 */

/*
 * Called at the end of a thread's execution
 * Place the dead thread on the dead_threads queue
 * and give control to the next thread
 */
int minithread_cleanup(arg_t arg) {
	queue_append(dead_queue, current);
	minithread_schedule();
	return 0;
}


/*
 * Scheduler
 */
int minithread_schedule() {
	minithread_t old = current;
	if ( alarm_has_ready() && old != idle ) {
		current = idle;
	} else if(multilevel_queue_length(ready_queue)>0) {
		minithread_age(ready_queue);
		multilevel_queue_dequeue(ready_queue, PRIORITY_SHORT, &current);
	} else {
		current = idle;
	}

	if ( current->priority == PRIORITY_SHORT ) {
		current_quanta_end = ticks + SHORT_QUANTA;
	} else {
		current_quanta_end = ticks + LONG_QUANTA;
	}
	minithread_switch(&(old->sp), &(current->sp));
	return 0;
}


/*
 * Idle / System thread body
 */
int minithread_idle() {
	while ( multilevel_queue_length(ready_queue) || queue_length(stop_queue) 
		|| queue_length(dead_queue) || alarm_has_ready() ) {
		while ( alarm_has_ready() ) {
			alarm_fire_next();
		}
		while ( queue_length(dead_queue) ) {
			minithread_t kill_thread;
			queue_dequeue(dead_queue, &kill_thread);
			minithread_free(kill_thread);
		}
		if ( multilevel_queue_length(ready_queue) ) {
			minithread_schedule();
		}
	}
	
	return 0;
}


/*
 * Clock Interrupt Handler
 */
void minithread_clock_handler(void *arg) {
	if ( ticks >= current_quanta_end ) {
		minithread_schedule();
	}
}


/*
 * Thread Awaken Callback - for sleep with timeout
 */
void minithread_awaken_callback(arg_t arg) {
	minithread_start( (minithread_t)arg );
}


/*
 * Thread Age - function that is used to promote threads
 * of a certain age to the next priority level, which
 * ensures that lower priority threads still get a chance
 * to use the CPU even when there are many high priority
 * threads
 */
int minithread_age(multilevel_queue_t queue) {
	minithread_t thread = NULL;
	multilevel_queue_peak(queue, PRIORITY_LONG, (any_t*)&thread);
	while( thread && ((ticks - thread->age) >= PROMOTE_AGE) ) {
		multilevel_queue_dequeue(queue, PRIORITY_LONG, (any_t*)&thread);
		// reset priority but age stays the same
		thread->priority = PRIORITY_SHORT;
		multilevel_queue_enqueue(queue, PRIORITY_SHORT, thread);
		multilevel_queue_peak(queue, PRIORITY_LONG, (any_t*)&thread);
	}
	return 0;
}
