/* network test program 6

   send messages back and forth between two processes on different computers

   usage: network6 [<hostname>]
   if no hostname is supplied, will wait for a packet before sending the first
   packet; if a hostname is given, will send and then receive. 
   the receive-first copy must be started first!
*/

#include "defs.h"
#include "minithread.h"
#include "minimsg.h"
#include "synch.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256
#define MAX_COUNT 100
#define INIT_RECEIVE_ID 5


char* hostname;
network_address_t remote_addr;
unsigned short remote_id;
miniport_t port = NULL;
miniport_t to = NULL;
char f_done_receive = 0;
char f_done_transmit = 0;


int receive_first(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;
  miniport_t from;

  minithread_fork(transmit_second, NULL);

  port = miniport_local_create(INIT_RECEIVE_ID);

  for (i=0; i<MAX_COUNT; i++) {
    length = BUFFER_SIZE;
    minimsg_receive(port, &from, buffer, &length);
	if (i == 0) {
		minimsg_get_addr(from, remote_addr);
		minimsg_get_id(from, &remote_id);
		to = miniport_remote_create(remote_addr, remote_port);
	}
    printf("%s", buffer);
    miniport_destroy(from);
  }

  f_done_receive = 1;
  if (f_done_transmit) {
    miniport_destroy(port);
    miniport_destroy(to);
  }

  return 0;
}


int transmit_second(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;

  while ( !to ) {
	minithread_yeild();
  }

  for (i=0; i<MAX_COUNT; i++) {
    printf("Sending packet %d.\n", i+1);
    sprintf(buffer, "Received packet %d.\n", i+1);
    length = strlen(buffer) + 1;
    minimsg_send(port, to, buffer, length);
  }

  f_done_transmit = 1;
  if (f_done_receive) {
    miniport_destroy(port);
    miniport_destroy(to);
  }

  return 0;
}



int transmit_first(int* arg) {
  char buffer[BUFFER_SIZE];
  int length = BUFFER_SIZE;
  int i;

  AbortOnCondition(network_translate_hostname(hostname, remote_addr) < 0,
		   "Could not resolve hostname, exiting.");
  remote_id = INIT_RECEIVE_ID;

  port = miniport_local_create(10); // this can be changed to anything - receiver will get send back to this id
  to = miniport_remote_create(remote_addr, remote_id);

  minithread_fork(receive_second, NULL);

  for (i=0; i<MAX_COUNT; i++) {
    printf("Sending packet %d.\n", i+1);
    sprintf(buffer, "Received packet %d.\n", i+1);
    length = strlen(buffer) + 1;
    minimsg_send(port, dest, buffer, length);
  }

  f_done_transmit = 1;
  if (f_done_receive) {
    miniport_destroy(port);
    miniport_destroy(to);
  }

  return 0;
}


int receive_second(int* arg) {
  char buffer[BUFFER_SIZE];
  int length = BUFFER_SIZE;
  int i;
  miniport_t from;

  for (i=0; i<MAX_COUNT; i++) {
    minimsg_receive(port, &from, buffer, &length);
    printf("%s", buffer);
    miniport_destroy(from);
  }

  f_done_receive = 1;
  if (f_done_transmit) {
    miniport_destroy(port);
    miniport_destroy(to);
  }

  return 0;
}


#ifdef WINCE
void main(void){
  READCOMMANDLINE
#else /* WINNT code */
main(int argc, char** argv) {
#endif

  if (argc > 1) {
		hostname = argv[1];
		minithread_system_initialize(transmit_first, NULL);
  }
  else {
		minithread_system_initialize(receive_first, NULL);
  }
  _CrtDumpMemoryLeaks();
}
