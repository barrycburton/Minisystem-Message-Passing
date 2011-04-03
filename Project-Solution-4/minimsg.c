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
	minimsg_port_t default_id;
	directory_t post_office;
};


/* mailbox data structure
 */
typedef struct minimsg_mailbox* minimsg_mailbox_t;
struct minimsg_mailbox {
	minimsg_port_t port;
	directory_t correspondents;
	queue_t msg_arrived;
	semaphore_t msg_available;
};


/* correspondent data structure - used by mailbox to
 * track information on each correspondent port -
 * that is each port it is sending to and / or
 * receiving from.
 */
typedef struct minimsg_corresp* minimsg_corresp_t;
struct minimsg_corresp {
	minimsg_mailbox_t parent;
	minimsg_port_t contact;
	network_address_t remote;
	minimsg_msgid_t last_rcvd;
	minimsg_msgid_t last_sent;
	minimsg_msg_t pending;
	alarm_id_t pending_timeout;
	semaphore_t rsp_available;
	queue_t rsp_arrived;
	queue_t waiting;
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
	minimsg_t msg_system = malloc(sizeof(struct minimsg));
	minimsg_mailbox_t box;

	dbgprintf("Initializing minimsg system...\n");

	msg_system->post_office = directory_new();

	network_initialize(&(minimsg_net_packet_handler));

	box = minimsg_mbox_create();
	msg_system->default_id = box->port;
	directory_add(msg_system->post_office, box->port, box);

	return msg_system;
}


/* shut down and cleanup up message passing
 * system. called from minithread_system_cleanup()
 */
int minimsg_system_cleanup(minimsg_t msg_system) {
	network_cleanup();

	directory_iterate(msg_system->post_office, &minimsg_mbox_iterate_free, 0, NULL);

	directory_destroy(msg_system->post_office);

	free(msg_system);

	dbgprintf("...minimsg system cleaned up.\n");
	return 0;
}


/* create a local mailbox (port),
 * return the port id
 */
minimsg_port_t minimsg_port_create() {
	minimsg_mailbox_t box = minimsg_mbox_create();
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	directory_add(minithread_msg_system()->post_office, box->port, box);
	set_interrupt_level(old_int);

	return box->port;
}


/* tear down port and release all resources
 */
int minimsg_port_destroy(minimsg_port_t minimsg_port) {
	minimsg_mailbox_t box;
	interrupt_level_t old_int = set_interrupt_level(DISABLED);
	if ( box = minimsg_remove_mbox(minimsg_port) ) {
		set_interrupt_level(old_int);
		
		minimsg_mbox_free(box);

		return 0;
	}
	set_interrupt_level(old_int);
	return -1;
}


/* return the id for the automatically created system port
 * (this is created at minimsg system start and exists for
 * it's lifetime - it should not be destroyed by the
 * application.)
 */
minimsg_port_t minimsg_system_port(void) {
	return minithread_msg_system()->default_id;
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
	if ( msg_len < MAX_MSG_SIZE && msg_len > 0 && msg ) {
		minimsg_corresp_t corresp;
		interrupt_level_t old_int = set_interrupt_level(DISABLED);

		if ( corresp = minimsg_get_port_corresp(from, to) ) {
			minimsg_msg_t send_msg;

			send_msg = minimsg_msg_create(from, to, msg_len, msg, response);

			minimsg_corresp_send_msg(corresp, send_msg);

			set_interrupt_level(old_int);

			return 0;
		}
		set_interrupt_level(old_int);
	}
	return -1;
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
	if ( msg && buffer_len_p && *buffer_len_p > 0 ) {
		minimsg_mailbox_t box;
		
		interrupt_level_t old_int = set_interrupt_level(DISABLED);

		if ( box = minimsg_get_mbox(me) ) {
			minimsg_msg_t rcv_msg;

			semaphore_P(box->msg_available);

			set_interrupt_level(DISABLED);

			queue_dequeue(box->msg_arrived, &rcv_msg);

			set_interrupt_level(old_int);

			minimsg_msg_extract(rcv_msg, msg, buffer_len_p, from_p, id_p);

			if ( from_p && *from_p == MINIMSG_SYSTEM_PORT_BCAST_ID ) {
				*from_p = rcv_msg->header.reply_to;
			}

			minimsg_msg_free(rcv_msg);

			return 0;
		}
		set_interrupt_level(old_int);
	}
	return -1;
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
	if ( msg_len < MAX_MSG_SIZE && msg_len > 0 && msg && buffer_len_p && *buffer_len_p > 0 ) {
		minimsg_corresp_t corresp;
		interrupt_level_t old_int = set_interrupt_level(DISABLED);

		if ( corresp = minimsg_get_port_corresp(me, to) ) {
			minimsg_msg_t send_msg, rcv_msg;

			send_msg = minimsg_msg_create(me, to, msg_len, msg, 0);
			
			minimsg_corresp_send_rpc(corresp, send_msg, &rcv_msg);

			set_interrupt_level(old_int);

			minimsg_msg_extract(rcv_msg, msg, buffer_len_p, NULL, NULL);
			minimsg_msg_free(rcv_msg);
			
			return 0;
		}
		set_interrupt_level(old_int);
	}

	return -1;
}




/*
 * UTILITY FUNCTION DEFINITIONS
 */

/* begin msg object fns */

minimsg_msg_t minimsg_msg_create(minimsg_port_t from, minimsg_port_t to, int msg_len, minimsg_data_t msg, minimsg_msgid_t response) {
	minimsg_msg_t new_msg = malloc(sizeof(struct minimsg_msg));
	new_msg->net_header.system_id = MINIMSG_GROUP_ID;
	new_msg->net_header.net_type = MINIMSG_NET_TYPE_DATA;
	new_msg->net_header.to = to;
	new_msg->header.from = from;
	new_msg->header.this_id = 0;
	new_msg->header.reply_to = response;
	new_msg->header.msg_len = msg_len;
	memcpy(new_msg->body, msg, msg_len);
	return new_msg;
}


minimsg_msg_t minimsg_msg_clone(minimsg_msg_t msg) {
	minimsg_msg_t new_msg = malloc(sizeof(struct minimsg_msg));
	new_msg->net_header.system_id = msg->net_header.system_id;
	new_msg->net_header.net_type = msg->net_header.net_type;
	new_msg->net_header.to = msg->net_header.to;
	new_msg->header.from = msg->header.from;
	new_msg->header.this_id = msg->header.this_id;
	new_msg->header.reply_to = msg->header.reply_to;
	new_msg->header.msg_len = msg->header.msg_len;
	memcpy(new_msg->body, msg->body, msg->header.msg_len);
	return new_msg;
}


int minimsg_msg_extract(minimsg_msg_t msg, minimsg_data_t buffer, int *buffer_len_p, minimsg_port_t *from_p, minimsg_msgid_t *id_p) {
	int len = (msg->header.msg_len < *buffer_len_p) ? msg->header.msg_len : *buffer_len_p;
	memcpy(buffer, msg->body, len);
	*buffer_len_p = len;
	if ( from_p ) {
		*from_p = msg->header.from;
	}
	if ( id_p ) {
		*id_p = msg->header.this_id;
	}
	return 0;
}


int minimsg_msg_free(minimsg_msg_t msg) {
	free(msg);
	return 0;
}


int minimsg_msg_iterate_free(any_t val_cur, any_t val) {
	minimsg_msg_free(val_cur);
	return 0;
}



/* begin minimsg layer */

minimsg_mailbox_t minimsg_get_mbox(minimsg_port_t port) {
	minimsg_mailbox_t box;
	if ( directory_get(minithread_msg_system()->post_office, port, &box) != 0 ) {
		box = NULL;
	}
	return box;
}


minimsg_mailbox_t minimsg_remove_mbox(minimsg_port_t port) {
	minimsg_mailbox_t box;
	if ( directory_remove(minithread_msg_system()->post_office, port, &box) != 0 ) {
		box = NULL;
	}
	return box;
}


minimsg_corresp_t minimsg_get_port_corresp(minimsg_port_t this_port, minimsg_port_t other_port) {
	minimsg_mailbox_t box;
	minimsg_corresp_t corresp = NULL;
	box = minimsg_get_mbox(this_port);
	if ( box ) {
		corresp = minimsg_mbox_get_corresp(box, other_port);
	}
	return corresp;
}



/* begin mbox layer */

minimsg_mailbox_t minimsg_mbox_create(void) {
	minimsg_mailbox_t box = malloc(sizeof(struct minimsg_mailbox));
	
	box->port = network_reserve_next_token();

	box->msg_arrived = queue_new();
	box->msg_available = semaphore_create();
	semaphore_initialize(box->msg_available, 0);

	box->correspondents = directory_new();

	return box;
}


int minimsg_mbox_free(minimsg_mailbox_t box) {

	queue_iterate(box->msg_arrived, minimsg_msg_iterate_free, NULL);

	queue_free(box->msg_arrived);

	semaphore_destroy(box->msg_available);

	directory_iterate(box->correspondents, &minimsg_corresp_iterate_free, 0, NULL);

	directory_destroy(box->correspondents);

	free(box);

	return 0;
}


int minimsg_mbox_iterate_free(key_t key_cur, any_t val_cur, key_t key, any_t val) {
	return minimsg_mbox_free((minimsg_mailbox_t)val_cur);
}


int minimsg_mbox_deliver_msg(minimsg_mailbox_t box, minimsg_msg_t msg) {
	queue_append(box->msg_arrived, msg);
	semaphore_V(box->msg_available);
	return 0;
}


minimsg_corresp_t minimsg_mbox_get_corresp(minimsg_mailbox_t box, minimsg_port_t other_port) {
	minimsg_corresp_t corresp;
	if ( directory_get(box->correspondents, other_port, &corresp) != 0 ) {
		/* first received from this, create corresp */
		corresp = minimsg_corresp_create(box, other_port);
		directory_add(box->correspondents, other_port, corresp);
	}

	return corresp;
}



/* begin corresp layer */

minimsg_corresp_t minimsg_corresp_create(minimsg_mailbox_t parent, minimsg_port_t corresp_id) {
	minimsg_corresp_t corresp = malloc(sizeof(struct minimsg_corresp));
	corresp->parent = parent;
	corresp->contact = corresp_id;
	network_address_zero(corresp->remote);
	corresp->last_rcvd = 0;
	corresp->last_sent = 0;
	corresp->pending = NULL;
	corresp->pending_timeout = NULL;
	corresp->waiting = queue_new();
	corresp->rsp_arrived = queue_new();
	corresp->rsp_available = semaphore_create();
	semaphore_initialize(corresp->rsp_available, 0);

	return corresp;
}


int minimsg_corresp_free(minimsg_corresp_t corresp) {
	semaphore_destroy(corresp->rsp_available);
	if ( corresp->pending_timeout ) {
		alarm_deregister(corresp->pending_timeout);
	}
	minimsg_msg_free(corresp->pending);
	queue_iterate(corresp->rsp_arrived, minimsg_msg_iterate_free, NULL);
	queue_free(corresp->rsp_arrived);
	queue_iterate(corresp->waiting, minimsg_msg_iterate_free, NULL);
	queue_free(corresp->waiting);
	free(corresp);
	return 0;
}


int minimsg_corresp_iterate_free(key_t key_cur, any_t val_cur, key_t key, any_t val) {
	return minimsg_corresp_free((minimsg_corresp_t)val_cur);
}


int minimsg_corresp_send_msg(minimsg_corresp_t to, minimsg_msg_t msg) {
	minimsg_corresp_t local = minimsg_get_port_corresp(to->contact, to->parent->port);

	to->last_sent++;
	msg->header.this_id = to->last_sent;
	
	if ( local ) {
		/* local corresp */
		minimsg_corresp_deliver_msg(local, msg);
	} else {
		/* corresp on another machine */
		if ( to->pending ) {
			queue_append(to->waiting, msg);
		} else {
			to->pending = msg;
			minimsg_net_send_to_corresp(to);
		}
	}

	return 0;
}


int minimsg_corresp_send_rpc(minimsg_corresp_t corresp, minimsg_msg_t query, minimsg_msg_t *response) {
	minimsg_msgid_t id;
	minimsg_msg_t temp;
	minimsg_corresp_t local = minimsg_get_port_corresp(corresp->contact, corresp->parent->port);
	
	corresp->last_sent++;
	id = query->header.this_id = corresp->last_sent;

	if ( local ) {
		/* local corresp */
		minimsg_corresp_deliver_msg(local, query);
	} else {
		/* corresp on another machine */
		if ( corresp->pending ) {
			queue_append(corresp->waiting, query);
		} else {
			corresp->pending = query;
			minimsg_net_send_to_corresp(corresp);
		}
	}

	temp = NULL;
	while ( !temp ) {
		semaphore_P(corresp->rsp_available);
		set_interrupt_level(DISABLED);
		queue_dequeue(corresp->rsp_arrived, &temp);
		if ( temp->header.reply_to != id ) {
			queue_append(corresp->rsp_arrived, temp);
			temp = NULL;
			semaphore_V(corresp->rsp_available);
			set_interrupt_level(DISABLED);
		}
	}

	*response = temp;

	return 0;
}


int minimsg_corresp_deliver_msg(minimsg_corresp_t corresp, minimsg_msg_t msg) {
	/* save msg id */
	corresp->last_rcvd = msg->header.this_id;

	if ( 0 == msg->header.reply_to || msg->header.from == MINIMSG_SYSTEM_PORT_BCAST_ID ) {
		/* normal message */
		minimsg_mbox_deliver_msg(corresp->parent, msg);

	} else {
		/* rpc response */
		queue_append(corresp->rsp_arrived, msg);
		semaphore_V(corresp->rsp_available);
	}

	return 0;
}



/* begin net layer */

/* network interrupt handler has interrupt handler
 * signature, single pointer argument is a pointer
 * to a network interrupt arg structure, which is
 * just a packet.
 */
void minimsg_net_packet_handler(void *int_arg) {
	if ( ((minimsg_msg_t)((network_interrupt_arg_t)int_arg)->buffer)->net_header.system_id == MINIMSG_GROUP_ID ) {
		/* from same group */
		if ( ((minimsg_msg_t)((network_interrupt_arg_t)int_arg)->buffer)->net_header.net_type == MINIMSG_NET_TYPE_ACK ) {
			/* control */
			minimsg_net_ack_handler((minimsg_net_ack_t)((network_interrupt_arg_t)int_arg)->buffer,
				((network_interrupt_arg_t)int_arg)->addr);
		} else if ( ((minimsg_msg_t)((network_interrupt_arg_t)int_arg)->buffer)->net_header.net_type == MINIMSG_NET_TYPE_DATA ) {
			/* data */
			minimsg_net_data_handler((minimsg_msg_t)((network_interrupt_arg_t)int_arg)->buffer,
				((network_interrupt_arg_t)int_arg)->addr);
		}
	}
	free(int_arg);
}


void minimsg_net_ack_handler(minimsg_net_ack_t packet, network_address_t addr) {
	minimsg_corresp_t corresp;
	if ( corresp = minimsg_get_port_corresp(packet->net_header.to, packet->header.from) ) {
		/* to port on this machine */
	
		/* make sure we have address */
		network_address_copy(addr, corresp->remote);

		if ( corresp->pending && corresp->pending->header.this_id == packet->header.reply_to ) {
			/* in response to message that is still pending */
			alarm_deregister(corresp->pending_timeout);
			corresp->pending_timeout = NULL;
			minimsg_msg_free(corresp->pending);
			corresp->pending = NULL;
			if ( queue_length(corresp->waiting) ) {
				/* there are waiting messages */
				queue_dequeue(corresp->waiting, &corresp->pending);
				minimsg_net_send_to_corresp(corresp);
			}
		}
	}
}


void minimsg_net_data_handler(minimsg_msg_t packet, network_address_t addr) {
	minimsg_corresp_t corresp;
	if ( corresp = minimsg_get_port_corresp(packet->net_header.to == MINIMSG_SYSTEM_PORT_BCAST_ID ? minithread_msg_system()->default_id : packet->net_header.to, packet->net_header.to == MINIMSG_SYSTEM_PORT_BCAST_ID ? packet->net_header.to : packet->header.from) ) {
		/* to port on this machine */
		dbgprintf("RCV: %d\n", *((int*)packet->body));
			
		/* send ack */
		minimsg_net_send_ack(addr, packet->header.from, packet->header.this_id, packet->net_header.to);

		if ( packet->net_header.to == MINIMSG_SYSTEM_PORT_BCAST_ID ) {
			packet->header.reply_to = packet->header.from;
			packet->header.from = packet->net_header.to;
			packet->net_header.to = minithread_msg_system()->default_id;
		}

		if ( corresp->last_rcvd < packet->header.this_id ) {
			/* new msg */
			minimsg_msg_t msg = minimsg_msg_clone(packet);
		
			/* save address */
			network_address_copy(addr, corresp->remote);

			/* have corresp do delivery */
			minimsg_corresp_deliver_msg(corresp, msg);
		}
	}
}


void minimsg_net_timeout_handler(arg_t timeout_arg) {
	minimsg_corresp_t corresp = (minimsg_corresp_t)timeout_arg;
	minimsg_net_send_to_corresp(corresp);
}


int minimsg_net_send_to_corresp(minimsg_corresp_t corresp) {
	network_address_t zero;
	network_address_zero(zero);
	if ( network_address_same(corresp->remote, zero) || corresp->contact == MINIMSG_SYSTEM_PORT_BCAST_ID ) {
		network_bcast_pkt(sizeof(struct minimsg_net_header) + sizeof(struct minimsg_header) + corresp->pending->header.msg_len, (char*)corresp->pending);
	} else {
		network_send_pkt(corresp->remote, sizeof(struct minimsg_net_header) + sizeof(struct minimsg_header) + corresp->pending->header.msg_len, (char*)corresp->pending);
	}
	dbgprintf("SEND: %d\n", *((int*)corresp->pending->body));
	alarm_register(MINIMSG_ACK_TIMEOUT, minimsg_net_timeout_handler, (arg_t)corresp, &corresp->pending_timeout);

	return 0;
}


int minimsg_net_send_ack(network_address_t addr, minimsg_port_t replying_to, minimsg_msgid_t concerning, minimsg_port_t me) {
	minimsg_net_ack_t ack = malloc(sizeof(struct minimsg_net_ack));

	ack->net_header.system_id = MINIMSG_GROUP_ID;
	ack->net_header.net_type = MINIMSG_NET_TYPE_ACK;
	ack->net_header.to = replying_to;
	ack->header.from = me;
	ack->header.reply_to = concerning;
	
	network_send_pkt(addr, sizeof(struct minimsg_net_ack), (char*)ack);

	free(ack);

	return 0;
}
