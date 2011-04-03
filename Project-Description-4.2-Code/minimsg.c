/*
 * minimsg.c - implements the message passing library
 */


/*
 * INCLUDES
 */
#include "defs.h"
#include "minimsg_private.h"
#include "minithread_private.h"
#include "network.h"
#include "queue.h"
#include "synch.h"
#include "alarm.h"
#include "directory.h"



/*
 * INTERNAL TYPES &
 * DATA STRUCTURES
 */

#define MINIMSG_ACK_TIMEOUT (500)
#define MINIMSG_MAX_TRIES (5)

typedef char minimsg_data_buf;


/* net packet type */
typedef enum minimsg_net_type minimsg_net_type_t;
enum minimsg_net_type {
	MINIMSG_NET_TYPE_DATA,
	MINIMSG_NET_TYPE_ACK
};
	

/* msg network header
 */
typedef struct minimsg_net_header *minimsg_net_header_t;
struct minimsg_net_header {
	short system_id;
	minimsg_port_t to;
	minimsg_net_type_t net_type;
};


/* msg header
 */
typedef struct minimsg_header *minimsg_header_t;
struct minimsg_header {
	minimsg_port_t from;
	minimsg_msgid_t this_id;
	minimsg_msgid_t reply_to;
	int msg_len;
};


/* network control packet structure
 */
typedef struct minimsg_net_ack *minimsg_net_ack_t;
struct minimsg_net_ack {
	struct minimsg_net_header net_header;
	struct minimsg_header header;
};


/* msg structure - this is designed so that
 * it can be used locally and as the data portion
 * of a packet. when used locally, simply ignore
 * the net_header. when using as packet, remember
 * that the packet length is not the same as the
 * body length (which is the value stored in the
 * header).
 */
typedef struct minimsg_msg *minimsg_msg_t;
struct minimsg_msg {
	struct minimsg_net_header net_header;
	struct minimsg_header header;
	minimsg_data_buf body[MAX_MSG_SIZE];
};


/* minimsg system data structure
 */
struct minimsg {

};


/* mailbox data structure
 */
typedef struct minimsg_mailbox* minimsg_mailbox_t;
struct minimsg_mailbox {

};


/* correspondent data structure - used by mailbox to
 * track information on each correspondent port -
 * that is each port it is sending to and / or
 * receiving from.
 */
typedef struct minimsg_corresp* minimsg_corresp_t;
struct minimsg_corresp {

};



/*
 * UTILITY FUNCTION DECLARATIONS
 */

/* msg object */
minimsg_msg_t minimsg_msg_create(minimsg_port_t from, minimsg_port_t to, int msg_len, minimsg_data_t msg, minimsg_msgid_t response);
int minimsg_msg_extract(minimsg_msg_t msg, minimsg_data_t buffer, int *buffer_len_p, minimsg_port_t *from_p, minimsg_msgid_t *id_p);
int minimsg_msg_free(minimsg_msg_t msg);
int minimsg_msg_iterate_free(any_t val_cur, any_t val);

/* minimsg layer */
minimsg_mailbox_t minimsg_get_mbox(minimsg_port_t port);
minimsg_mailbox_t minimsg_remove_mbox(minimsg_port_t port);
minimsg_corresp_t minimsg_get_port_corresp(minimsg_port_t this_port, minimsg_port_t other_port);

/* mbox layer */
minimsg_mailbox_t minimsg_mbox_create(void);
int minimsg_mbox_free(minimsg_mailbox_t mbox);
int minimsg_mbox_iterate_free(key_t key_cur, any_t val_cur, key_t key, any_t val);
int minimsg_mbox_deliver_msg(minimsg_mailbox_t box, minimsg_msg_t msg);
minimsg_corresp_t minimsg_mbox_get_corresp(minimsg_mailbox_t box, minimsg_port_t other_port);

/* corresp layer */
minimsg_corresp_t minimsg_corresp_create(minimsg_mailbox_t parent, minimsg_port_t corresp_id);
int minimsg_corresp_free(minimsg_corresp_t corresp);
int minimsg_corresp_iterate_free(key_t key_cur, any_t val_cur, key_t key, any_t val);
int minimsg_corresp_send_msg(minimsg_corresp_t to, minimsg_msg_t msg);
int minimsg_corresp_send_rpc(minimsg_corresp_t corresp, minimsg_msg_t query, minimsg_msg_t *response);
int minimsg_corresp_deliver_msg(minimsg_corresp_t corresp, minimsg_msg_t msg);

/* net layer */
void minimsg_net_packet_handler(void *int_arg);
void minimsg_net_ack_handler(minimsg_net_ack_t packet, network_address_t addr);
void minimsg_net_data_handler(minimsg_msg_t packet, network_address_t addr);
void minimsg_net_timeout_handler(arg_t timeout_arg);
int minimsg_net_send_to_corresp(minimsg_corresp_t corresp);
int minimsg_net_send_ack(network_address_t addr, minimsg_port_t replying_to, minimsg_msgid_t concerning, minimsg_port_t me);



/*
 * INTERFACE FUNCTION DEFINITIONS
 */

/* set up message passing system
 * called from minithread_system_initialize()
 */
minimsg_t minimsg_system_initialize() {

}


/* shut down and cleanup up message passing
 * system. called from minithread_system_cleanup()
 */
int minimsg_system_cleanup(minimsg_t msg_system) {

}


/* create a local mailbox (port),
 * return the port id
 */
minimsg_port_t minimsg_port_create() {

}


/* tear down port and release all resources
 */
int minimsg_port_destroy(minimsg_port_t minimsg_port) {

}


/* return the id for the automatically created system port
 * (this is created at minimsg system start and exists for
 * it's lifetime - it should not be destroyed by the
 * application.)
 */
minimsg_port_t minimsg_system_port(void) {

}


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
extern int minimsg_send(minimsg_port_t from, minimsg_port_t to, int msg_len, minimsg_data_t msg, minimsg_msgid_t response) {

}


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
extern int minimsg_receive(minimsg_port_t me, minimsg_data_t msg, int *buffer_len_p, minimsg_port_t *from_p, minimsg_msgid_t *id_p) {

}


/* do an rpc operation from a port to another port.
 * this involves sending a message to the correspondent
 * port, then waiting to receive a reply message from
 * that same port. this also blocks until the reply
 * is received.
 * buffer_len_p is read to determine the size of the msg buffer
 * buffer_len_p is written to output the size of the return message
 */
int minimsg_rpc(minimsg_port_t me, minimsg_port_t to, int msg_len, minimsg_data_t msg, int *buffer_len_p) {

}




/*
 * UTILITY FUNCTION DEFINITIONS
 */

/* begin msg object fns */

minimsg_msg_t minimsg_msg_create(minimsg_port_t from, minimsg_port_t to, int msg_len, minimsg_data_t msg, minimsg_msgid_t response) {

}


minimsg_msg_t minimsg_msg_clone(minimsg_msg_t msg) {

}


int minimsg_msg_extract(minimsg_msg_t msg, minimsg_data_t buffer, int *buffer_len_p, minimsg_port_t *from_p, minimsg_msgid_t *id_p) {

}


int minimsg_msg_free(minimsg_msg_t msg) {

}


int minimsg_msg_iterate_free(any_t val_cur, any_t val) {

}



/* begin minimsg layer */

minimsg_mailbox_t minimsg_get_mbox(minimsg_port_t port) {

}


minimsg_mailbox_t minimsg_remove_mbox(minimsg_port_t port) {

}


minimsg_corresp_t minimsg_get_port_corresp(minimsg_port_t this_port, minimsg_port_t other_port) {

}



/* begin mbox layer */

minimsg_mailbox_t minimsg_mbox_create(void) {

}


int minimsg_mbox_free(minimsg_mailbox_t box) {

}


int minimsg_mbox_iterate_free(key_t key_cur, any_t val_cur, key_t key, any_t val) {

}


int minimsg_mbox_deliver_msg(minimsg_mailbox_t box, minimsg_msg_t msg) {

}


minimsg_corresp_t minimsg_mbox_get_corresp(minimsg_mailbox_t box, minimsg_port_t other_port) {

}



/* begin corresp layer */

minimsg_corresp_t minimsg_corresp_create(minimsg_mailbox_t parent, minimsg_port_t corresp_id) {

}


int minimsg_corresp_free(minimsg_corresp_t corresp) {

}


int minimsg_corresp_iterate_free(key_t key_cur, any_t val_cur, key_t key, any_t val) {

}


int minimsg_corresp_send_msg(minimsg_corresp_t to, minimsg_msg_t msg) {

}


int minimsg_corresp_send_rpc(minimsg_corresp_t corresp, minimsg_msg_t query, minimsg_msg_t *response) {

}


int minimsg_corresp_deliver_msg(minimsg_corresp_t corresp, minimsg_msg_t msg) {

}



/* begin net layer */

/* network interrupt handler has interrupt handler
 * signature, single pointer argument is a pointer
 * to a network interrupt arg structure, which is
 * just a packet.
 */
void minimsg_net_packet_handler(void *int_arg) {

}


void minimsg_net_ack_handler(minimsg_net_ack_t packet, network_address_t addr) {

}


void minimsg_net_data_handler(minimsg_msg_t packet, network_address_t addr) {

}


void minimsg_net_timeout_handler(arg_t timeout_arg) {

}


int minimsg_net_send_to_corresp(minimsg_corresp_t corresp) {

}


int minimsg_net_send_ack(network_address_t addr, minimsg_port_t replying_to, minimsg_msgid_t concerning, minimsg_port_t me) {

}
