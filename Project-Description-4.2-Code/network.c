/*
 * network.c:
 *	This module paints the unix socket interface a pretty color.
 */


#define _WIN32_WINNT 0x0400

/* INCLUDES */

#include <windows.h>
#include <winsock.h>
#include <winerror.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "network.h"
#include "machineprimitives.h"
#include "interrupts_private.h"
#include "minimsg_private.h"



/* DATA STRUCTURE DEFINITIONS */

struct address_info {
	SOCKET sock;
	struct sockaddr_in sin;
	char pkt[MAX_NETWORK_PKT_SIZE];
};

struct subport_node {
	unsigned short port_num;
	struct subport_node* next;
};


/* CONSTANT DEFINES */
#define NETWORK_PORT_START (41500 + MINIMSG_GROUP_ID)
#define NETWORK_SIMULATE_ERROR (1)
#define NETWORK_ERROR_LOSS (.1)
#define NETWORK_BCAST_ADDRESS "132.236.227.255"



/* STATIC INSTANCE DATA */

unsigned short my_udp_port;
unsigned short other_udp_port;
double loss_rate;
char synthetic_network;
char network_up;
static HANDLE network_poll_done = NULL;
static HANDLE subports_done = NULL;
struct subport_node *registered_subports;
WSADATA winsock_version_data;
struct address_info if_info;
static tas_lock_t initialized = 0;
static network_address_t broadcast_addr = { 0 };
static network_address_t my_addr;
static int process_id;
static int last_id; 


/* INTERNAL FUNCTION DECLARATIONS */

int send_pkt(network_address_t dest_address, int data_len, char *data);
int sockaddr_to_network_address(struct sockaddr_in* sin, network_address_t addr);
int network_address_to_sockaddr(network_address_t addr, struct sockaddr_in* sin);
int network_set_udp_ports(unsigned short myportnum, unsigned short otherportnum);
int network_set_synthetic_params(double loss);
int WINAPI network_poll(void* arg);
int start_network_poll(interrupt_handler_t, SOCKET*);



/* EXTERNAL FUNCTION IMPLEMENTATIONS */

/* network_initialize should be called before clock interrupts start
 * happening (or with clock interrupts disabled).  The initialization
 * procedure returns 0 on success, -1 on failure.  The function
 * handler(data) is called when a network packet arrives.
 */
int network_initialize(interrupt_handler_t network_handler) {
	int arg = 1;
	int ret = 0;
	char name[32];
	char hostname[64];
	struct sockaddr_in msin;

	/* initialise the NT socket library, inexplicably required by NT */
	assert(WSAStartup(MAKEWORD(2, 0), &winsock_version_data) == 0);

	if (atomic_test_and_set(&initialized)) {
		return -1;
	}
	network_up = 1;
	registered_subports = NULL;

	memset(&if_info, 0, sizeof(if_info));

	if_info.sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (if_info.sock < 0)  {
		perror("socket");
		return -1;
	}

	assert(gethostname(hostname, 64) == 0);
	network_translate_hostname(hostname, my_addr);
	network_address_to_sockaddr(my_addr, &msin);

	other_udp_port = NETWORK_PORT_START;
	my_udp_port = NETWORK_PORT_START;
	if_info.sin.sin_family = SOCK_DGRAM;
	if_info.sin.sin_addr.s_addr = msin.sin_addr.s_addr;
	my_udp_port--;
	do {
		my_udp_port++;
		if_info.sin.sin_port = htons(my_udp_port);
	} while ( ((ret = bind(if_info.sock, (struct sockaddr *) &if_info.sin, sizeof(if_info.sin))) < 0)
		&& (WSAEADDRINUSE == WSAGetLastError() ) );

	if ( ret < 0 ) {
		/* error */
		kprintf("Error: code %ld.\n", GetLastError());
		AbortOnError(0);
		perror("bind");
		return -1;
	}

	/* save my address */
	sockaddr_to_network_address(&(if_info.sin), my_addr);

	if ( NETWORK_PORT_START != my_udp_port ) {
		/* not the default port, so register ourself with default */
		struct sockaddr_in sin;
		sin.sin_port = htons(other_udp_port);
		sin.sin_family = if_info.sin.sin_family;
		sin.sin_addr.s_addr = if_info.sin.sin_addr.s_addr;
		/* broadcast to default */
		sockaddr_to_network_address(&sin, broadcast_addr);

		if_info.pkt[0] = 1;
		sendto(if_info.sock, if_info.pkt, 1, 0, (struct sockaddr*)&sin, sizeof(sin));
	} else {
		/* i am default, broadcast to myself */
		network_address_copy(my_addr, broadcast_addr);
	}

	if ( MINIMSG_IN_CSUG ) {
		/* actually, in the lab, so broadcast to everyone */
		network_translate_hostname(NETWORK_BCAST_ADDRESS, broadcast_addr);
	}

	/* set for fast reuse */
	assert(setsockopt(if_info.sock, SOL_SOCKET, SO_REUSEADDR, (char *) &arg, sizeof(int)) == 0);

	/* initialize broadcast */
	assert(setsockopt(if_info.sock, SOL_SOCKET, SO_BROADCAST, (char *) &arg, sizeof(int)) == 0);
		
	sprintf_s(name, 32, "npm %d", GetCurrentProcessId());
	network_poll_done = CreateMutex(NULL, FALSE, NULL);
	sprintf_s(name, 32, "spm %d", GetCurrentProcessId());
	subports_done = CreateMutex(NULL, FALSE, NULL);

	/* Interrupts are handled through the caller's handler. */
	start_network_poll(network_handler, &if_info.sock);

	loss_rate = NETWORK_ERROR_LOSS;
	synthetic_network = NETWORK_SIMULATE_ERROR;

	process_id = last_id = 0;
	
	/* Print My Name & Address */
	dbgprintf("This Host: %s\n", hostname);
	dbgprintf("Address: ");
	network_address_print(my_addr);
	dbgprintf("\n");

	return 0;
}


/* shutdown the networking system and cleans up all
 * used resources
 */
int network_cleanup(void) {
	network_up = 0;

	if ( NETWORK_PORT_START != my_udp_port ) {
		/* not the default port, so deregister ourself with default */
		struct sockaddr_in sin;
		sin.sin_port = htons(other_udp_port);
		sin.sin_family = if_info.sin.sin_family;
		sin.sin_addr.s_addr = if_info.sin.sin_addr.s_addr;

		if_info.pkt[0] = 0;
		sendto(if_info.sock, if_info.pkt, 1, 0, (struct sockaddr*)&sin, sizeof(sin));
	} else if ( registered_subports != NULL ) {
		/* this is default port, so have to keep forwarding broadcasts
		 * 'til all other virtual network interfaces on this machine close down
		 */
		WaitOnObject(subports_done);
		ReleaseMutex(subports_done);
	}
	
	shutdown(if_info.sock, 2);

	closesocket(if_info.sock);
	
	/* wait until the polling loop exits so we know
	 * that no more interrupts will be sent.
	 */
	WaitOnObject(network_poll_done);
	ReleaseMutex(network_poll_done);

	deregister_interrupt(NETWORK_INTERRUPT_TYPE);

	atomic_clear(&initialized);

	return 0;
}


/* reserve the next available globally unique 
 * token, and then return the value of this token
 */
int network_reserve_next_token(void) {
	if ( !process_id ) {
		process_id = if_info.sin.sin_addr.S_un.S_un_b.s_b4 * 2000 + (my_udp_port - other_udp_port) * 200 + 2;
		last_id = process_id;
	} else {
		last_id++;
	}
	return last_id;
}


/* sends raw data to all destinations (anyone who is listening).
 * it returns the number of bytes sent if it was able to
 * successfully send the data or -1 otherwise.
 */
int network_bcast_pkt(int data_len, char* data) {
	if (synthetic_network) {
		if( rand() < (loss_rate * RAND_MAX) ) {
			dbgprintf("Packet dropped.\n");
			return data_len;
		}
	}

	return send_pkt(broadcast_addr, data_len, data);
}


/* sends raw data to the specified destination.
 * it returns the number of bytes sent if it was able to
 * successfully send the data or -1 otherwise.
 */
int network_send_pkt(network_address_t dest_address, int data_len, char *data) {
	if (synthetic_network) {
		if( rand() < (loss_rate * RAND_MAX) ) {
			dbgprintf("Packet dropped.\n");
			return (data_len);
		}
	}

	return send_pkt(dest_address, data_len, data);
}


/* translate hostname into a network address object. note
 * that hostname may actually be an ip address or hostname.
 */
int network_translate_hostname(char* hostname, network_address_t address) {
	struct hostent* host;
	unsigned long iaddr;

	if( isalpha(hostname[0]) ) {
		host = gethostbyname(hostname);
		if (host == NULL) {
			return -1;
		} else {
			address[0] = (long) *((int *) host->h_addr);
			address[1] = (long) htons(other_udp_port);
			return 0;
		}
	} else {
		iaddr = inet_addr(hostname);
		address[0] = iaddr;
		address[1] = (long) htons(other_udp_port);
		return 0;
	}
}


/* Returns the network_address_t that can be used
 * to send a packet to the caller's address space.  Note that
 * an address space can send a packet to itself by specifying the result of
 * network_get_my_address() as the dest_address to network_send_pkt.
 */
int network_get_my_address(network_address_t my_address) {
	my_address[0] = if_info.sin.sin_addr.s_addr;
	my_address[1] = (long)if_info.sin.sin_port;
	return 0;
}


/* Copy address "original" to address "copy".
 */
int network_address_copy(network_address_t original, network_address_t copy) {
	copy[0] = original[0];
	copy[1] = original[1];
	return 0;
}


/* Zero the address, so as to make it invalid
 */
int network_address_zero(network_address_t addr) {
	addr[0]=addr[1]=0;
	return 0;
}


/* Compare two addresses, return 1 if same, 0 otherwise.
 */
int network_address_same(network_address_t a, network_address_t b) {
	return (a[0] == b[0] && a[1] == b[1]);
}


/* Print an address
 */
int network_address_print(network_address_t address) {
	char name[40];
	int length = 40;
	struct in_addr ipaddr;
	char* textaddr;
	int addrlen;

	ipaddr.s_addr = address[0];
	textaddr = inet_ntoa(ipaddr);
	addrlen = strlen(textaddr);

	if (length >= addrlen + 7) {
		strcpy_s(name, length, textaddr);
		name[addrlen] = ':';
		sprintf_s(name+addrlen+1, length - addrlen - 1, "%d", ntohs((short) address[1]));
		dbgprintf("%s", name);
		return 0;
	} else if ( length >= addrlen + 1 ) {
		strcpy_s(name, length, textaddr);
		dbgprintf("%s", name);
		return 0;
	}
	return -1;
}



/* INTERNAL FUNCTION IMPLEMENTATIONS */

int send_pkt(network_address_t dest_address, int data_len, char *data) {
	int cc;
	struct sockaddr_in sin;
	char* bufp;
	int sz;


	/* sanity checks */
	if ( data_len < 0 || data_len > MAX_NETWORK_PKT_SIZE ) {
		return 0;
	}

	/*
	 * put data in packet
	 */
	bufp = if_info.pkt;

	memcpy(bufp, data, data_len);
	sz = data_len;

	bufp += sz;
	if ( dest_address == broadcast_addr ) {
		bufp[0] = 2;
		sz += 1;
	} else {
		bufp[0] = 3;
		sz += 1;
	}

	/* if broadcast, send to any registered subports
	 */
	if ( dest_address == broadcast_addr && registered_subports ) {
		struct subport_node *temp = registered_subports;
		struct sockaddr_in local_addr;

		/* address will be correct, no need to put in body */
		bufp[0] = 3;

		/* set up dest address struct */
		local_addr.sin_family = if_info.sin.sin_family;
		local_addr.sin_addr.s_addr = if_info.sin.sin_addr.s_addr;

		while ( temp ) {
			local_addr.sin_port = temp->port_num;
			sendto(if_info.sock, if_info.pkt, sz, 0, (struct sockaddr*)&local_addr, sizeof(local_addr));
			temp = temp->next;
		}

		/* reset this for others */
		bufp[0] = 2;
	}

	network_address_to_sockaddr(dest_address, &sin);
	cc = sendto(if_info.sock, if_info.pkt, sz, 0, (struct sockaddr *) &sin, sizeof(sin));

	return cc;
}


int sockaddr_to_network_address(struct sockaddr_in* sin, network_address_t addr) {
	addr[0] = sin->sin_addr.s_addr;
	addr[1] = sin->sin_port;
	return 0;
}


int network_address_to_sockaddr(network_address_t addr, struct sockaddr_in* sin) {
	memset(sin, 0, sizeof(*sin));
	sin->sin_addr.s_addr = addr[0];
	sin->sin_port = (short)addr[1];
	sin->sin_family = SOCK_DGRAM;
	return 0;
}


int network_set_udp_ports(unsigned short myportnum, unsigned short otherportnum) {
	my_udp_port = myportnum;
	other_udp_port = otherportnum;
	return 0;
}


int network_set_synthetic_params(double loss) {
	synthetic_network = 1;
	loss_rate = loss;
	return 0;
}


/* 
 * receive incoming packets from the specified socket, translate their 
 * addresses to network_address_t type, and call the user-supplied handler
 *
 * a network interrupt disables interrupts, so that we can cleanly return.
 * if interrupts were not disabled, and we were hit by a clock interrupt, it
 * would not be possible to return until we returned to the original stack.
 * this is inelegant, but we are constrained by ignorance of the minithread
 * struct format!
*/
int WINAPI network_poll(void* arg) {
	SOCKET* s;
	network_interrupt_arg_t packet;
	struct sockaddr_in addr;
	int fromlen = sizeof(struct sockaddr_in);

	s = (SOCKET *) arg;

	WaitOnObject(network_poll_done);
	WaitOnObject(subports_done);

	while ( network_up ) {
		/* we rely on run_user_handler to destroy this data structure */
		packet = (network_interrupt_arg_t) malloc(sizeof(struct network_interrupt_arg));
		assert(packet != NULL);

		/* do receive */
		packet->size = recvfrom(*s, packet->buffer, MAX_NETWORK_PKT_SIZE, 0,
								(struct sockaddr *)&addr, &fromlen);
		/* check for errors */
		if (packet->size <= 0) {
			int err = WSAGetLastError();
			if( err == 10054){
				dbgprintf("NET: Message sent to unavailable host.\n");
				free(packet);
				continue;
			} else if ( err == WSAEINTR || err == WSAESHUTDOWN ) {
				/* blocking operation canceled */
				free(packet);
				continue;
			} else {
				dbgprintf("NET: Error, %d.\n", err);
				free(packet);
				AbortOnCondition(1,"Crashing.");
			}
		}

		/* make sure it's not from me */
		if ( addr.sin_addr.s_addr == if_info.sin.sin_addr.s_addr &&
			addr.sin_port == if_info.sin.sin_port ) {
			free(packet);
			continue;
		}

		/* check for virtual network interface control packet */
		if ( 1 == packet->size && addr.sin_addr.s_addr == if_info.sin.sin_addr.s_addr ) {
			if ( 1 == packet->buffer[0] ) {
				/* register */
				struct subport_node *new_node = malloc(sizeof(struct subport_node));
				new_node->port_num = addr.sin_port;
				new_node->next = registered_subports;
				registered_subports = new_node;
				free(packet);
				continue;
			} else if ( 0 == packet->buffer[0] ) {
				/* deregister */
				struct subport_node *temp = registered_subports, *prev = NULL;
				while ( temp && temp->port_num != addr.sin_port ) {
					prev = temp;
					temp = temp->next;
				}
				if ( temp ) {
					/* found it */
					if ( prev ) {
						prev->next = temp->next;
					} else {
						registered_subports = temp->next;
					}
					free(temp);
				}
				free(packet);
				if ( !registered_subports ) {
					ReleaseMutex(subports_done);
				}
				continue;
			}
		}

		/* if this is broadcast message */
		else if ( 2 == packet->buffer[packet->size - 1] ) {
			if ( registered_subports ) {
				/* forward this packet */
				struct subport_node *temp = registered_subports;
				struct sockaddr_in sin;
				int new_size;
				char *buf;
		
				/* set up dest address struct */
				sin.sin_family = if_info.sin.sin_family;
				sin.sin_addr.s_addr = if_info.sin.sin_addr.s_addr;

				/* append real address info to message */
				buf = &(packet->buffer[packet->size - 1]);
				memcpy(buf, &(addr.sin_addr.s_addr), sizeof(addr.sin_addr.s_addr));
				buf += sizeof(addr.sin_addr.s_addr);
				memcpy(buf, &(addr.sin_port), sizeof(addr.sin_port));
				buf += sizeof(addr.sin_port);
				buf[0] = 2;
				new_size = packet->size + sizeof(addr.sin_addr.s_addr) + sizeof(addr.sin_port);

				while ( temp ) {
					sin.sin_port = temp->port_num;
					if ( sin.sin_port != addr.sin_port ) {
						sendto(if_info.sock, packet->buffer, packet->size, 0, (struct sockaddr*)&sin, sizeof(sin));
					}
					temp = temp->next;
				}
			} else if ( NETWORK_PORT_START != my_udp_port ) {
				/* packet has been forwarded, so fix address */
				char *buf = &(packet->buffer[packet->size - 1 - sizeof(addr.sin_port) - sizeof(addr.sin_addr.s_addr)]);
				memcpy(&(addr.sin_addr.s_addr), buf, sizeof(addr.sin_addr.s_addr));
				buf += sizeof(addr.sin_addr.s_addr);
				memcpy(&(addr.sin_port), buf, sizeof(addr.sin_port));
				packet->size -= (sizeof(addr.sin_addr.s_addr) + sizeof(addr.sin_port));
			}

			/* regardless, skip over type info */
			packet->size--;
		}

		/* if this is normal message */
		else if ( 3 == packet->buffer[packet->size - 1] ) {
			/* just skip over message type info */
			packet->size--;
		}

		assert(fromlen == sizeof(struct sockaddr_in));
		sockaddr_to_network_address(&addr, packet->addr);

		/* send arg to users' handler */
		send_interrupt(NETWORK_INTERRUPT_TYPE, (void*)packet);
	}

	dbgprintf("...network interrupts stopped.\n");

	ReleaseMutex(network_poll_done);

	return 0;
}


/* 
 * start polling for network packets. this is separate so that clock interrupts
 * can be turned on without network interrupts. however, this function requires
 * that clock_init has been called!
 */
int start_network_poll(interrupt_handler_t network_handler, SOCKET* s) {
	HANDLE network_thread = NULL; /* NT thread to check for incoming packets */
	DWORD id;

	dbgprintf("Starting network interrupts...\n");

	register_interrupt(NETWORK_INTERRUPT_TYPE, network_handler, INTERRUPT_DEFER);

	/* create network thread */
	network_thread = CreateThread(NULL, 0, network_poll, s, 0, &id); 
	assert(network_thread != NULL);

	return 0;
}
