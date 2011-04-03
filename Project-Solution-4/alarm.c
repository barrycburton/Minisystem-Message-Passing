/*
 * alarm.c - implements the alarm library
 */


/*
 * INCLUDES
 */
#include "defs.h"

#include "interrupts.h"

#include "alarm_private.h"

#include "minithread_private.h"

#include "priority_queue.h"


struct alarm_entry {
	alarm_callback func;
	arg_t arg;
	long time;
};
typedef struct alarm_entry *alarm_entry_t;

struct alarm {
	priority_queue_t registered;
};


/* 
 * register an alarm to go off in delay milliseconds. the library
 * will fire the alarm after at least delay milliseconds have
 * passed by calling func with the single argument arg. the alarm may
 * be fired arbitrarily later than delay milliseconds, but alarms will
 * be fired in order (absolute time registered + registered delay should
 * never be lower for a subsequent alarm than for the previous alarm).
 * if requested, return out an alarm id which uniquely identifies this
 * alarm. This id can be used to later deregister the alarm.
 * on success, return 0. on failure, return -1.
 */
int alarm_register(int delay, alarm_callback func, arg_t arg, alarm_id_t *id_p) {
	alarm_t me = minithread_alarm_system();
	alarm_entry_t al = malloc(sizeof(struct alarm_entry));
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	al->time = ticks + ((delay * MILLISECOND) / PERIOD);
	al->func = func;
	al->arg = arg;
	priority_queue_enqueue(me->registered, al->time, al);
	dbgprintf("ALARM: reg %d\n", priority_queue_length(me->registered));
	set_interrupt_level(old_int);
	if (id_p) {
		*id_p = al;
	}
	return 0;
}


/* 
 * deregister the alarm corresponing to id.
 * on success, return 0. on failure, return -1.
 */
int alarm_deregister(alarm_id_t id) {
	alarm_t me = minithread_alarm_system();
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	if ( priority_queue_delete(me->registered, id) == 0 ) {
		dbgprintf("ALARM: dereg %d\n", priority_queue_length(me->registered));
		set_interrupt_level(old_int);
		free(id);
		return 0;
	} else {
		set_interrupt_level(old_int);
		return -1;
	}
}


/* 
 * initialize the alarm system.
 * on success, return 0. on failure, return -1.
 */
alarm_t alarm_system_initialize(void) {
	alarm_t obj = malloc(sizeof(struct alarm));
	dbgprintf("Initializing alarm system...\n");
	obj->registered = priority_queue_new(0);
	return obj;
}


/* 
 * return true (1) if there are registered alarms,
 * or return false (0) if there are none registered
 */
int alarm_has_remaining(void) {
	alarm_t me = minithread_alarm_system();
	if ( priority_queue_length(me->registered) ) {
		return 1;
	}
	return 0;
}


/* 
 * return true (1) if the next alarm is ready,
 * or return false (0) if it is not ready or
 * if it does not exist
 */
int alarm_has_ready(void) {
	alarm_t me = minithread_alarm_system();
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	if ( alarm_has_remaining() ) {
		alarm_entry_t next;
		priority_queue_peak(me->registered, &next);
		if ( next->time <= ticks ) {
			set_interrupt_level(old_int);
			return 1;
		}
	}
	set_interrupt_level(old_int);
	return 0;
}


/* 
 * fire the next ready alarm
 */
int alarm_fire_next(void) {
	alarm_t me = minithread_alarm_system();
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	if ( alarm_has_ready() ) {
		alarm_entry_t next;
		priority_queue_dequeue(me->registered, &next);
		dbgprintf("ALARM: fire %d\n", priority_queue_length(me->registered));
		set_interrupt_level(ENABLED);
		(*(next->func))(next->arg);
		free(next);
	}
	set_interrupt_level(old_int);
	return 0;
}


/*
 * clean up the alarm system
 * on success, return 0. on failure, return -1.
 */
int alarm_system_cleanup(alarm_t alarm_system) {
	priority_queue_free(alarm_system->registered);
	free(alarm_system);
	dbgprintf("...alarm system cleaned up.\n");
	return 0;
}
