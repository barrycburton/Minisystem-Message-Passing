#ifndef __NETWORK_H__
#define __NETWORK_H__

/*
 * network.h:
 *	Low-level network interface.
 *
 *	This interface defines a low-level network interface for sending and
 *	receiving packets between pseudo-network interfaces located on the 
 *  same or different hosts.
 */

#include "interrupts.h"


#define MAX_NETWORK_PKT_SIZE (8192)

/* treat this as opaque - always use the interface functions.
 * you never know when this definition will change, so don't
 * do anything that relies on it remaining static (otherwise
 * your code would break).
 */
typedef unsigned long network_address_t[2];


/* a structure of this type is passed as an argument to
 * the network interrupt handler. addr gives the address
 * of the sender, buffer holds the message (which will
 * contain the minimsg header as well the minimsg data),
 * and size tells how many bytes long the message is.
 * it is the interrupt handler's responsibility to free this
 * memory (and your program should not try to access this
 * memory outside of the interrupt handler), so you will need
 * to do some memory copying in places.
 */
typedef struct network_interrupt_arg {
  network_address_t addr;
  char buffer[MAX_NETWORK_PKT_SIZE];
  int size;
} *network_interrupt_arg_t;



/* network_initialize should be called before clock interrupts start
 * happening (or with clock interrupts disabled).  The initialization
 * procedure returns 0 on success, -1 on failure.  The function
 * handler(data) is called when a network packet arrives.
 */
int network_initialize(interrupt_handler_t network_handler);


/* shutdown the networking system and cleans up all
 * used resources
 */
int network_cleanup(void);


/* reserve the next available globally unique 
 * token, and then return the value of this token
 */
int network_reserve_next_token(void);


/* sends raw data to all destinations (anyone who is listening).
 * it returns the number of bytes sent if it was able to
 * successfully send the data or -1 otherwise.
 */
int network_bcast_pkt(int data_len, char* data);


/* sends raw data to the specified destination.
 * it returns the number of bytes sent if it was able to
 * successfully send the data or -1 otherwise.
 */
int network_send_pkt(network_address_t dest_address, int data_len, char * data);


/* translate hostname into a network address object. note
 * that hostname may actually be an ip address or hostname.
 */
int network_translate_hostname(char* hostname, network_address_t address);


/* Returns the network_address that can be used
 * to send a packet to the caller's address space.  Note that
 * an address space can send a packet to itself by specifying the result of
 * network_get_my_address() as the dest_address to network_send_pkt.
 */
int network_get_my_address(network_address_t my_address);


/* Copy address "original" to address "copy".
 */
int network_address_copy(network_address_t original, network_address_t copy);


/* Zero the address, so as to make it invalid
 */
int network_address_zero(network_address_t addr);


/* Compare two addresses, return 1 if same, 0 otherwise.
 */
int network_address_same(network_address_t a, network_address_t b);


/* Print an address
 */
int network_address_print(network_address_t address);



#endif __NETWORK_H_
