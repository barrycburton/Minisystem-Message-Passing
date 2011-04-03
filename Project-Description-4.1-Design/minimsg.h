#ifndef __MINIMSG_H__
#define __MINIMSG_H__


/* Interface for minimsg
 * message passing system
 */

/* typedef for a port id (mailbox name)
 */
typedef int minimsg_port_t;
typedef char *minimsg_data_t;


/* create a local mailbox (port),
 * return the port id
 */
extern miniport_t minimsg_port_create();


/* tear down port and release all resources
 */
extern int minimsg_port_destroy(miniport_t miniport);


/* send message from a port to another port. this
 * returns immediately, and though best effort is
 * made, there is no guarantee of when or if the
 * message is received. however, it is guaranteed
 * that subsequent sends from the same port to the
 * same destination port will only be received after
 * this message, if both are received.
 */
extern int minimsg_send(miniport_t from, miniport_t to, int msg_len, minimsg_data_t msg);


/* receive message from the specified port. this
 * will block until a message addressed to this
 * port comes in, if there are no queued messages
 * that were already received.
 * msg must be a pre-allocated buffer
 * buffer_len_p is read to determine the size of the msg buffer
 * buffer_len_p is written to output the size of the return message
 */
extern int minimsg_receive(miniport_t me, minimsg_data_t msg, int *buffer_len_p);


/* do an rpc operation from a port to another port.
 * this involves sending a message to the destination
 * port, then waiting to receive a reply message from
 * that same port. this also blocks until the reply
 * is received.
 * buffer_len_p is read to determine the size of the msg buffer
 * buffer_len_p is written to output the size of the return message
 */
extern int minimsg_rpc(miniport_t me, miniport_t to, int msg_len, minimsg_data_t msg, int *buffer_len_p);


#endif __MINIMSG_H__
