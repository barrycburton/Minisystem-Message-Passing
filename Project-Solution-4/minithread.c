/*
 * minithread implementation
 */

#include "defs.h"
#include "machineprimitives.h"
#include "interrupts.h"

#include "queue.h"
#include "synch.h"
#include "multilevel_queue.h"
#include "minithread_private.h"


/* System Configuration Constants
 *
 * PERIOD is defined in interrupts.h and this tells the
 * the number of microseconds between clock interrupts
 * We will be dealing with units of millseconds, so use
 * the MILLISECOND define as a multiplier
 *
 * ***** ticks ***** is global variable which is declared in
 * interrupts.h and which tells how many PERIODs have elapsed
 * since the clock interrupts were started. it is your friend.
 */


// define the length of a short in terms of periods
#define SHORT_QUANTA (2)

// define the length of a long quanta in terms of short quanta
#define LONG_QUANTA_SHORTS (2)

// define how much time (microseconds) is in a short quanta
#define SHORT_QUANTA_MS (SHORT_QUANTA * PERIOD)

// define how many periods are in a long quanta
#define LONG_QUANTA (LONG_QUANTA_SHORTS * SHORT_QUANTA)

// define how much time (microseconds) is in a long quanta
#define LONG_QUANTA_MS (LONG_QUANTA * PERIOD)

// define priority of short running thread
#define PRIORITY_SHORT (0)

// define priority of long running thread
#define PRIORITY_LONG (1)

// define age (in periods) at which long running thread is promoted to short
#define PROMOTE_AGE (2 * LONG_QUANTA_SHORTS)



// Data Structures and System State

struct minithread {
	int id;
	stack_pointer_t sp;
	stack_pointer_t sb;
	int priority;
	long age;
};

// The currently executing minithread
minithread_t current = NULL;
minithread_t idle = NULL;

// The ready queue of threads
multilevel_queue_t ready_queue;

// the stopped queue
queue_t stop_queue;

// A queue of threads which have died, and need to be freed
queue_t dead_queue;

// The id of the last created thread
int last_id;

// the ticks at which the current quanta ends
long current_quanta_end;

minimsg_t msg_system;

alarm_t alarm_system;


// system state
//struct minisystem {

//};
//typedef struct minisystem* minisystem_t;


// Single allowed global for system state
//minisystem_t this_system;




/*
 * Internal Function Declarations
 * (definitions at bottom of file)
 */
int minithread_idle(void);
void minithread_awaken_callback(arg_t arg);
void minithread_clock_handler(void *arg);
int minithread_schedule(void);
int minithread_age(multilevel_queue_t queue);
int minithread_free(minithread_t thread);
int minithread_cleanup(arg_t arg);
int minithread_system_cleanup(void);



/*
 * interface functions
 */


/*
 *	Create and schedule a new thread of control. When the
 *  scheduler chooses the thread, it will start executing
 *  the function refered to by proc, with it being called
 *  with arg as the single argument.
 */	
minithread_t minithread_fork(proc_t proc, arg_t arg) {
	minithread_t new_thread = minithread_create(proc, arg);
	if (new_thread == NULL)
		return NULL;
	minithread_start(new_thread);
	return new_thread;
}


/*
 *	Create a new thread, but do not schedule it for execution.
 *  After start is called on the thread it will be added to
 *  the scheduler. After this, when the scheduler chooses the
 *  the thread, it will start executing the function refered
 *  to by proc, with it being called with arg as the single
 *  argument.
 */
minithread_t minithread_create(proc_t proc, arg_t arg) {
	minithread_t new_thread = (minithread_t)malloc(sizeof(struct minithread));
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	if (new_thread == NULL)
		return NULL;
	new_thread->id = ++last_id;
	new_thread->priority = PRIORITY_SHORT;
	minithread_allocate_stack(&(new_thread->sb), &(new_thread->sp));
	minithread_initialize_stack(&(new_thread->sp), proc, arg, minithread_cleanup, NULL);
	if ( proc ) {
		queue_append(stop_queue, new_thread);
	}
	set_interrupt_level(old_int);
	return new_thread;
}


/*
 *	Return handle (minithread_t) of calling thread.
 */
minithread_t minithread_self() {
	return current;
}


/*
 *  Return thread identifier of calling thread.
 */
int minithread_id() {
	return current->id;
}


/*
 *	Block the calling thread. This yields the
 *  processor but does not place the thread
 *  back into the scheduler. Note that the thread
 *  still exists and if another thread calls
 *  minithread_start, it will be placed back
 *  in the scheduler and once chosen will
 *  resume executing after this call.
 */
int minithread_stop() {
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	current->priority = PRIORITY_SHORT;
	queue_append(stop_queue, current);
	minithread_schedule();
	return 0;
}


/*
 *	Make thread runnable.
 */
int minithread_start(minithread_t thread) {
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	if ( 0 == queue_delete(stop_queue, thread) )
	{
		thread->age = ticks;
		multilevel_queue_enqueue(ready_queue, current->priority, thread);
	}
	set_interrupt_level(old_int);
	return 0;
}


/*
 *	Forces the caller to relinquish the processor and be put to the end of
 *	the ready queue.  Allows another thread to run.
 */
int minithread_yield() {
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	current->priority = PRIORITY_SHORT;
	current->age = ticks;
	multilevel_queue_enqueue(ready_queue, current->priority, current);
	minithread_schedule();
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
	minithread_stop();
	set_interrupt_level(old_int);
	return 0;
}


/*
 *	Initialize the system to run the first minithread at
 *	mainproc(mainarg).  This procedure should be called from your
 *	main program with the callback procedure and argument specified
 *	as arguments.
 */
int minithread_system_initialize(proc_t mainproc, arg_t mainarg) {
	dbgprintf("Initializing minisystem...\n");
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
	alarm_system = alarm_system_initialize();

	// start clock interrupts
	minithread_clock_init(minithread_clock_handler);
	// interrupts currently disabled

	// initialize message passing subsystem
	msg_system = minimsg_system_initialize();

	// schedule will call switch, which will enable interrupts
	minithread_schedule();

	// begin idle thread body
	minithread_idle();

	// system is shutting down now
	set_interrupt_level(DISABLED);

	// cleanup msg passing system
	minimsg_system_cleanup(msg_system);

	// stop clock interrupts
	minithread_clock_stop();

	// cleanup alarm system
	alarm_system_cleanup(alarm_system);

	// cleanup thread system
	minithread_system_cleanup();

	return 0;
}



/* 
 * Internal Functions (only used in c file, so input does not
 * have to be checked thoroughly, but function prototypes MUST NOT
 * be listed in header file.
 */


/*
 * Idle / System thread body
 */
int minithread_idle(void) {
	int i;
	while ( multilevel_queue_length(ready_queue) || queue_length(stop_queue) || queue_length(dead_queue) || alarm_has_remaining() ) {
			while ( alarm_has_ready() ) {
				alarm_fire_next();
			}
			while ( queue_length(dead_queue) ) {
				minithread_t kill_thread;
				interrupt_level_t old_int = set_interrupt_level(DISABLED);
				queue_dequeue(dead_queue, &kill_thread);
				set_interrupt_level(old_int);
				minithread_free(kill_thread);
			}
			if ( multilevel_queue_length(ready_queue) ) {
				interrupt_level_t old_int = set_interrupt_level(DISABLED);
				minithread_schedule();
			}
			i = 0;
			while ( i < 100000 ) {
				i++;
			}
	}
	return 0;
}


/*
 * Thread Awaken Callback - for sleep with timeout
 */
void minithread_awaken_callback(arg_t arg) {
	minithread_start( (minithread_t)arg );
}


/*
 * Clock Interrupt Handler
 */
void minithread_clock_handler(void *arg) {
	if ( ticks >= current_quanta_end ) {
		current->priority = PRIORITY_LONG;
		current->age = ticks;
		multilevel_queue_enqueue(ready_queue, current->priority, current);
		minithread_schedule();
	}
}


/*
 * Scheduler function
 */
int minithread_schedule(void) {
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
 * Thread Age - function that is used to promote threads
 * of a certain age to the next priority level, which
 * ensures that lower priority threads still get a chance
 * to use the CPU even when there are many high priority
 * threads
 */
int minithread_age(multilevel_queue_t queue) {
	minithread_t thread = NULL;
	int ret = multilevel_queue_peak(queue, PRIORITY_LONG, (any_t*)&thread);
	while( ret != -1 && thread && ((ticks - thread->age) >= PROMOTE_AGE) ) {
		if ( multilevel_queue_dequeue(queue, PRIORITY_LONG, (any_t*)&thread) != -1 ) {
			// reset priority but age stays the same
			thread->priority = PRIORITY_SHORT;
			multilevel_queue_enqueue(queue, PRIORITY_SHORT, thread);
		}
		ret = multilevel_queue_peak(queue, PRIORITY_LONG, (any_t*)&thread);
	}
	return 0;
}


/*
 * Free memory for the argument thread.
 */
int minithread_free(minithread_t thread) {
	if ( thread->sb ) {
		minithread_free_stack(thread->sb);
	}
	free(thread);
	return 0;
}


/*
 * Called at the end of a thread's execution
 * Place the dead thread on the dead_threads queue
 * and give control to the next thread
 */
int minithread_cleanup(arg_t arg) {
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	queue_append(dead_queue, current);
	minithread_schedule();
	return 0;
}


/*
 * Cleanup all system state.
 */
int minithread_system_cleanup(void) {
	multilevel_queue_free(ready_queue);
	queue_free(stop_queue);
	queue_free(dead_queue);
	minithread_free(idle);
	dbgprintf("...minisystem cleaned up and shut down.\n");
	return 0;
}


/*
 * Private Interface Functions
 */
minimsg_t minithread_msg_system(void) {
	return msg_system;
}

alarm_t minithread_alarm_system(void) {
	return alarm_system;
}
