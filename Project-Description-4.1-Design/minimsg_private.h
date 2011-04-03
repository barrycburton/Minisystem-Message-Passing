#ifndef __MINIMSG_PRIVATE_H__
#define __MINIMSG_PRIVATE_H__

/* private interface for minimsgs (forms the bottom interface
 * of minimsg that is accessed only by minithread.c)
 */

#include "minimsg.h"

/* type definition for minimsg system
 */
typedef struct minimsg* minimsg_t;


/* set up message passing system
 * called from minithread_system_initialize()
 */
minimsg_t minimsg_initialize();


/* shut down and cleanup up message passing
 * system. called from minithread_system_cleanup()
 */
int minimsg_cleanup(minimsg_t msg_system);


#endif __MINIMSG_PRIVATE_H__
