/* network test program 4

   similar to 3, but in reverse: two senders send to one receiver.
*/

#include "defs.h"
#include "minithread.h"
#include "minimsg.h"
#include "synch.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256
#define MAX_COUNT 80

miniport_t port;

int receive(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;
  miniport_t from;

  for (i=0; i<2*MAX_COUNT; i++) {
    length = BUFFER_SIZE;
    minimsg_receive(port, &from, buffer, &length);
    printf("%s", buffer);
    miniport_destroy(from);
  }

  miniport_destroy(port);

  return 0;
}

int transmit2(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;

  for (i=0; i<MAX_COUNT; i++) {
    printf("Sending packet %d from sender 2.\n", i+1);
    sprintf(buffer, "Count from sender 2 is %d.\n", i+1);
    length = strlen(buffer) + 1;
    minimsg_send(port, port, buffer, length);
    minithread_yield();
  }

  return 0;
}

int transmit1(int* arg) {
  char buffer[BUFFER_SIZE];
  int length;
  int i;
  minithread_t transmitter2;
  minithread_t receiver;

  port = miniport_local_create(0);

  transmitter2 = minithread_fork(transmit2, NULL);
  receiver = minithread_fork(receive, NULL);

  for (i=0; i<MAX_COUNT; i++) {
    printf("Sending packet %d from sender 1.\n", i+1);
    sprintf(buffer, "Count from sender 1 is %d.\n", i+1);
    length = strlen(buffer) + 1;
    minimsg_send(port, port, buffer, length);
    minithread_yield();
  }

  return 0;
}

main(int argc, char** argv) {
  minithread_system_initialize(transmit1, NULL);
  _CrtDumpMemoryLeaks();
}
