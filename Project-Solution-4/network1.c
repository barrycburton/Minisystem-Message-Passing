/* network test program 1

   sends and then receives one message using the same local port.
*/

#include "minithread.h"
#include "minimsg.h"
#include "synch.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


#define BUFFER_SIZE 256


miniport_t port;

char text[] = "Hello, world!\n";
int textlen=14;

int thread(int* arg) {
  char buffer[BUFFER_SIZE];
  int length = BUFFER_SIZE;
  miniport_t from;

  port = miniport_local_create(0);
  minimsg_send(port, port, text, textlen);
  minimsg_receive(port, &from, buffer, &length);
  printf("%s", buffer);

  miniport_destroy(from);
  miniport_destroy(port);

  return 0;
}

main(int argc, char** argv) {
  textlen = strlen(text) + 1;
  minithread_system_initialize(thread, NULL);
  _CrtDumpMemoryLeaks();
}
