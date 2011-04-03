/*
 * alarm_private.h - defines the data structures and utility functions
 * used to implement the alarm library which need to be shared
 * between minisystem components.
 */

#ifndef __ALARM_PRIVATE_H__
#define __ALARM_PRIVATE_H__



#include "alarm.h"



/* type definition for
 * alarm system
 */
typedef struct alarm *alarm_t;


/* 
 * initialize the alarm system.
 * on success, return 0. on failure, return -1.
 */
alarm_t alarm_system_initialize(void);


/* 
 * return true (1) if there are registered alarms,
 * or return false (0) if there are none registered
 */
int alarm_has_remaining(void);


/* 
 * return true (1) if the next alarm is ready,
 * or return false (0) if it is not ready or
 * if it does not exist
 */
int alarm_has_ready(void);


/* 
 * fire the next ready alarm
 * on success, return 0. on failure, return -1.
 */
int alarm_fire_next(void);


/*
 * clean up the alarm system
 * on success, return 0. on failure, return -1.
 */
int alarm_system_cleanup(alarm_t alarm_system);


#endif __ALARM_PRIVATE_H__
