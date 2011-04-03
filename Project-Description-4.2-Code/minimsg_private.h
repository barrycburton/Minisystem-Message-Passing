#ifndef __MINIMSG_PRIVATE_H__
#define __MINIMSG_PRIVATE_H__

/* private interface for minimsgs (forms the bottom interface
 * of minimsg that is accessed only by minithread.c)
 */


#include "minimsg.h"


/* this following should be defined to be a random number
 * from 0 to 255. it is used to make sure that while
 * debugging your projects, you do not step on other groups'
 * toes. (and vice versa of course).
 */
#error "set MINIMSG_GROUP_ID to random ID from 0 to 255 and remove this line"
#define MINIMSG_GROUP_ID

#define MINIMSG_IN_CSUG 0


/* type definition for minimsg system
 */
typedef struct minimsg* minimsg_t;


/* set up message passing system
 * called from minithread_system_initialize()
 */
minimsg_t minimsg_system_initialize();


/* shut down and cleanup up message passing
 * system. called from minithread_system_cleanup()
 */
int minimsg_system_cleanup(minimsg_t msg_system);


#endif __MINIMSG_PRIVATE_H__
