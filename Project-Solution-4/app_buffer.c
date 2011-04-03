/*
* Bounded buffer example. 
*
* Sample program that implements a single producer-single consumer
* system.
* 
* Change MAXCOUNT to vary the number of items produced by the producer.
*/
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

#include "minithread.h"
#include "synch.h"


#define BUFFER_SIZE 10

#define MAXCOUNT  100

int buffer[BUFFER_SIZE];
int size, head, tail;

semaphore_t empty;
semaphore_t full;

int consumer(int* arg) {
	int get_num, i;
	int count = 0;

	printf("Consumer thread started (id: %d).\n", minithread_id());

	while (count < *arg) {
		get_num = rand() % BUFFER_SIZE + 1;
		get_num = (get_num <= *arg - count) ? get_num : *arg - count;

		printf("Consumer wants to get %d items out of buffer ...\n", get_num);
		for (i=0; i<get_num; i++) {
			semaphore_P(empty);
			count = buffer[tail];
			printf("Consumer is taking %d out of buffer.\n", count);
			tail = (tail + 1) % BUFFER_SIZE;
			size--;
			semaphore_V(full);
		}
	}

	printf("%d items Consumed.\n", count);

	return 0;
}

int producer(int* arg) {
	int put_num, i;
	int count = 0;

	printf("Producer thread started (id: %d).\n", minithread_id());

	minithread_fork(consumer, arg);

	minithread_yield();

	while (count < *arg) {
		put_num = rand() % BUFFER_SIZE + 1;
		put_num = (put_num <= *arg - count) ? put_num : *arg - count;

		printf("Producer wants to put %d items into buffer ...\n", put_num);
		for (i=0; i<put_num; i++) {
			semaphore_P(full);
			buffer[head] = ++count;
			printf("Producer is putting %d into buffer.\n", count);
			head = (head + 1) % BUFFER_SIZE;
			size++;
			semaphore_V(empty);
		}
	}

	printf("%d items Produced.\n", count);

	return 0;
}


void main(void) {
	int maxcount = MAXCOUNT;

	printf("In main function.\n");

	size = head = tail = 0;
	empty = semaphore_create();
	semaphore_initialize(empty, 0);
	full = semaphore_create();
	semaphore_initialize(full, BUFFER_SIZE);

	minithread_system_initialize(producer, &maxcount);

	semaphore_destroy(full);
	semaphore_destroy(empty);

	// use printf to print to output window
	// allows program status to be logged without
	// cluttering up console output
	dbgprintf("Memory Leaks (If Any) Follow:\n");

	// Now print out memory leak report (this just works when
	// running in Debug mode in Visual Studio. output can be
	// found in the Output Pane, not the console window
	_CrtDumpMemoryLeaks();

	// Finally keep the Command Window open 'til enter is pressed
	system("pause");
}
