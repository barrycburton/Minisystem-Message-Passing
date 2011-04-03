/*
 * minithread_private.h - defines the data structures used by
 * other minisystem components
 */

#ifndef __MINITHREAD_PRIVATE_H__
#define __MINITHREAD_PRIVATE_H__


#include "minithread.h"
#include "minimsg_private.h"
#include "alarm_private.h"


minimsg_t minithread_msg_system(void);

alarm_t minithread_alarm_system(void);


#endif __MINITHREAD_PRIVATE_H__
