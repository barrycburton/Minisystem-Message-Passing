/* network test program 3

   spawns three threads and creates two ports. one thread acts as the sender
   and sends pairs of messages, one to each port, in a loop, with yields in
   between. each of the other threads is assigned a port, and reads messages
   out of it. both ports are local.
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

miniport_t port1;
miniport_t port2;

int receive1(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;
  miniport_t from;

  for (i=0; i<MAX_COUNT; i++) {
    length = BUFFER_SIZE;
    minimsg_receive(port1, &from, buffer, &length);
    printf("%s", buffer);
    miniport_destroy(from);
  }

  miniport_destroy(port1);

  return 0;
}

int receive2(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;
  miniport_t from;

  for (i=0; i<MAX_COUNT; i++) {
    length = BUFFER_SIZE;
    minimsg_receive(port2, &from, buffer, &length);
    printf("%s", buffer);
    miniport_destroy(from);
  }

  miniport_destroy(port2);

  return 0;
}

int transmit(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;
  minithread_t receiver1;
  minithread_t receiver2;

  port1 = miniport_local_create(0);
  port2 = miniport_local_create(1);

  receiver1 = minithread_fork(receive1, NULL);
  receiver2 = minithread_fork(receive2, NULL);

  for (i=0; i<MAX_COUNT; i++) {
    printf("Sending packet %d to receiver 1.\n", i+1);
    sprintf(buffer, "Count for receiver 1 is %d.\n", i+1);
    length = strlen(buffer) + 1;
    minimsg_send(port1, port1, buffer, length);
    minithread_yield();
    printf("Sending packet %d to receiver 2.\n", i+1);
    sprintf(buffer, "Count for receiver 2 is %d.\n", i+1);
    length = strlen(buffer) + 1;
    minimsg_send(port2, port2, buffer, length);
  }

  return 0;
}

main(int argc, char** argv) {
  minithread_system_initialize(transmit, NULL);
  _CrtDumpMemoryLeaks();
}
