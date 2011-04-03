#ifndef __MINIMSG_H__
#define __MINIMSG_H__


/* Interface for minimsg
 * message passing system
 */


/* messages must be less than this size
 * in order to be passed
 */
#define MAX_MSG_SIZE (5196)

#define MINIMSG_SYSTEM_PORT_BCAST_ID (1)

#define MINIMSG_UNDEFINED (0)


/* typedef for a port id (mailbox name),
 * msg id (for responses), and msg data
 */
typedef int minimsg_port_t;
typedef int minimsg_msgid_t;
typedef char *minimsg_data_t;


/* create a local mailbox (port),
 * return the port id
 */
extern minimsg_port_t minimsg_port_create(void);


/* tear down port and release all resources
 */
extern int minimsg_port_destroy(minimsg_port_t minimsg_port);


/* return the id for the automatically created system port
 * (this is created at minimsg system start and exists for
 * it's lifetime - it should not be destroyed by the
 * application.)
 */
extern minimsg_port_t minimsg_system_port(void);


/* send message from a port to another port. this
 * returns immediately, and though best effort is
 * made, there is no guarantee of when or if the
 * message is received. however, it is guaranteed
 * that subsequent sends from the same port to the
 * same destination port will only be received after
 * this message, if both are received.
 * if response is MINIMSG_UNDEFINED, then it is ignored,
 * otherwise, message is sent as RPC response to the
 * query with msgid response.
 */
extern int minimsg_send(minimsg_port_t from, minimsg_port_t to, int msg_len, minimsg_data_t msg, minimsg_msgid_t response);


/* receive message from the specified port. this
 * will block until a message addressed to this
 * port comes in, if there are no queued messages
 * that were already received.
 * msg must be a pre-allocated buffer
 * buffer_len_p is read to determine the size of the msg buffer
 * buffer_len_p is written to output the size of the return message
 * from_p may be NULL. if not, return out the message sender port
 * id_p may be NULL. if not, return out the message id
 */
extern int minimsg_receive(minimsg_port_t me, minimsg_data_t msg, int *buffer_len_p, minimsg_port_t *from_p, minimsg_msgid_t *id_p);


/* do an rpc operation from a port to another port.
 * this involves sending a message to the destination
 * port, then waiting to receive a reply message from
 * that same port. this also blocks until the reply
 * is received.
 * buffer_len_p is read to determine the size of the msg buffer
 * buffer_len_p is written to output the size of the return message
 */
extern int minimsg_rpc(minimsg_port_t me, minimsg_port_t to, int msg_len, minimsg_data_t msg, int *buffer_len_p);


#endif __MINIMSG_H__
