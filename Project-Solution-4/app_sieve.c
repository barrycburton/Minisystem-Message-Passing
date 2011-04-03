/*
* Sieve of Eratosthenes application for finding prime numbers
*
* This program will print out all the prime numbers less than
* or equal to MAXPRIME. It works via three different kinds of
* threads - a producer thread creates numbers and inserts them
* into a pipeline. A consumer consumes numbers that make it through
* the pipeline and prints them out as primes. It also creates a new
* filter thread for each new prime, which subsequently filters out
* all multiples of that prime from the pipe.
*
*/
#include "defs.h"
#include "minithread.h"
#include "synch.h"

#define MAXPRIME 1000


typedef struct {
	int value;
	semaphore_t produce;
	semaphore_t consume;
} channel_t;

typedef struct {
	channel_t* left;
	channel_t* right;
	int prime;
} filter_t;


enum whichThread {
	SINK,
	FILTER,
	SOURCE,
	GREEDY,
	SLEEPY
};


int max = MAXPRIME;
int last;
int sleepyDone;
int sinkDone;

int spin(int num, int id) {
	int i = 0;
	while ( i < (num) ) {
		i++;
		last = id;
	}
	return i;
}

/* greedy thread that only gets in the way */
int greedy(int* arg) {
	long count = 0;

	while (last != GREEDY || !sleepyDone) {
		if ( last != GREEDY ) {
			printf("Greedy has counted to %lu\n", count);
			last = GREEDY;
		}
		count++;
	}
	printf("Greedy exits.\n");
	return 0;
}

/* sleepy thread that takes its time */
int sleepy(int* arg) {
	last = SLEEPY;

	while ( !sinkDone ) {
		last = SLEEPY;
		minithread_sleep_with_timeout(1000);
		last = SLEEPY;
	}

	printf("Sleepy takes a 3 second nap.\n");

	minithread_sleep_with_timeout(3000);

	last = SLEEPY;

	sleepyDone = 1;

	printf("Sleepy wakes. All done.\n");

	minithread_sleep_with_timeout(1000);

	printf("Sleepy exits.\n");
	return 0;
}

/* produce all integers from 2 to max */
int source(int* arg) {
	channel_t* c = (channel_t *) arg;
	int i;

	for (i=2; i<=max; i++) {
		c->value = i;
		last = SOURCE;
		semaphore_V(c->consume);
		last = SOURCE;
		semaphore_P(c->produce);
		last = SOURCE;
	}

	c->value = -1;
	last = SOURCE;
	semaphore_V(c->consume);
	last = SOURCE;
	semaphore_V(c->consume);
	last = SOURCE;

	spin(1000, SOURCE);
	printf("Source exits.\n");
	return 0;
}

int filter(int* arg) {
	filter_t* f = (filter_t *) arg;
	int value = 0;
	int prime = f->prime;

	while ( value != -1 ) {
		last = FILTER;
		semaphore_P(f->left->consume);
		last = FILTER;
		value = f->left->value;
		semaphore_V(f->left->produce);
		last = FILTER;
		if ((value == -1) || (value % f->prime != 0)) {
			f->right->value = value;
			semaphore_V(f->right->consume);
			last = FILTER;
			semaphore_P(f->right->produce);
			last = FILTER;
		}
	}

	semaphore_V(f->right->consume);
	last = FILTER;
	semaphore_P(f->left->consume);
	last = FILTER;

	//cleanup left
	semaphore_destroy(f->left->consume);
	semaphore_destroy(f->left->produce);
	free(f->left);

	//cleanup self
	free(f);

	spin(1000, FILTER);
	printf("Filter %d exits.\n", prime);
	return 0;
}

int sink(int* arg) {
	channel_t* p = (channel_t *) malloc(sizeof(channel_t));
	int value;
	filter_t* f;
	int i;
	minithread_t sleepy_t;


	last = SINK;
	sinkDone = 0;
	sleepyDone = 0;
	

	printf("Will begin in 5 seconds");

	for(i=0; i * 500 < 5000; i++) {
		minithread_sleep_with_timeout(250);
		printf(".");
	}
	minithread_sleep_with_timeout(250);
	printf("Started.\n");

	sleepy_t = minithread_fork(sleepy, NULL);

	minithread_fork(greedy, NULL);

	p->produce = semaphore_create();
	semaphore_initialize(p->produce, 0);
	p->consume = semaphore_create();
	semaphore_initialize(p->consume, 0);

	minithread_fork(source, (int *) p);

	last = SINK;
	semaphore_P(p->consume);
	last = SINK;
	value = p->value;
	semaphore_V(p->produce);
	last = SINK;
	
	while ( value != -1 ) {
		last = SINK;
		spin(1000, SINK);
		printf("%d is prime.\n", value);
		
		f = (filter_t *) malloc(sizeof(filter_t));
		f->left = p;
		f->prime = value;

		p = (channel_t *) malloc(sizeof(channel_t));
		p->produce = semaphore_create();
		semaphore_initialize(p->produce, 0);
		p->consume = semaphore_create();
		semaphore_initialize(p->consume, 0);

		f->right = p;

		minithread_fork(filter, (int *) f);

		last = SINK;
		semaphore_P(p->consume);
		last = SINK;
		value = p->value;
		semaphore_V(p->produce);
	}
	last = SINK;
	semaphore_P(p->consume);
	last = SINK;

	// cleanup last channel
	semaphore_destroy(p->consume);
	semaphore_destroy(p->produce);
	free(p);

	sinkDone = 1;

	spin(1000, SINK);
	printf("Sink Exits.\n");
	return 0;
}

void main(void) {
	minithread_system_initialize(sink, NULL);

	// use dbgprintf to print to output window
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
