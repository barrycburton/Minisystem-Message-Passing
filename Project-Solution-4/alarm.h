/*
 * alarm.h - defines the interface for the alarm library
 */

#ifndef __ALARM_H__
#define __ALARM_H__


/* 
 * TYPE DEFINITIONS
 */

/* generic function argument */
typedef int *arg_t;

/* alarm callback function pointer */
typedef void(*alarm_callback)(arg_t);

/* alarm id type */
typedef void* alarm_id_t;



/*
 * INTERFACE FUNCTION DECLARATIONS
 */

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
extern int alarm_register(int delay, alarm_callback func, arg_t arg, alarm_id_t *id_p);


/* 
 * deregister the alarm corresponing to id.
 * on success, return 0. on failure, return -1.
 */
extern int alarm_deregister(alarm_id_t id);



#endif __ALARM_H__
