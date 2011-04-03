/*
 * test_queue.c - has a main function which implements
 * a simple application that excercise parts of the
 * queue API. note that these tests are far from
 * complete. also note that as written, test failures
 * are cascading, in the sense that a failure at one
 * point will likely cause all remaining tests to fail,
 * so when fixing problems always start at the top.
 * tests are more concise to write this way, and after
 * components are debugged it is good to make check that
 * the system performs as expected even after multifaceted
 * interactions, but the first tests for complex systems
 * should isolate components as much as is possible.
 *
 * One thing that is always important is clear concise output.
 * Remember that other programmers will want to be able to
 * easily use and understand your tests.
 */

// Constant Defines
#define NUM_APPENDS (10)	// correctness relies on this value
							// being greater than 9
#define ERR_STRN_LEN (32)

// Platform Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Set Up Memory Leak Debugging
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Local Includes
#include "queue.h"
#include "defs.h"


int main(void) {
	queue_t q = NULL;
	int i;
	int ret;
	char *error;

	// malloc and fill in error string
	error = malloc(ERR_STRN_LEN*sizeof(char));
	if ( !error ) {
		// no memory?
		return -1;
	}
	strcpy_s(error, ERR_STRN_LEN, "Error Encountered: %s\n\n");

	// Now tell the (human) tester our plan
	printf("Running Tests on Queue ADT.\n");
	printf("Errors will be output. Successes will be silent\n\n");


	// try creating a queue
	q = queue_new();
	if ( !q ) {
		printf(error, "Create Failed");
		free(error);
		return -1;
	}

	// how many items?
	ret = queue_length(q);
	if ( ret == -1 ) {
		printf(error, "Length Returned Failure Code");
	} else if ( ret != 0 ) {
		printf(error, "Brand New (Empty) Queue Thinks It Has Items");
	}

	// shouldn't be able to dequeue
	ret = queue_dequeue(q, (any_t*)&i);
	if ( ret != -1 ) {
		printf(error, "Dequeue Thinks It Dequeue Something From An Empty Queue");
	}

	// try appending several items
	for (i = 1; i <= NUM_APPENDS; i++) {
		if ( -1 == queue_append(q, (any_t)i) ) {
			printf(error, "Append Returned Failure Code");
		}
		dbgprintf("here is i: %d", i);
	}

	// now see if dequeue is correct (returns 1)
	ret = queue_dequeue(q, (any_t*)&i);
	if ( ret == -1 ) {
		printf(error, "Dequeue Returned Failure Code");
		if ( i != 0 ) {
			printf(error, "Dequeue Failed but Did Not Return NULL in Item");
		}
	} else if ( i != 1 ) {
		printf(error, "Dequeue Returned Wrong Item");
	}

	// try dequeue again (returns 2)
	ret = queue_dequeue(q, (any_t*)&i);
	if ( ret == -1 ) {
		printf(error, "Dequeue Returned Failure Code");
	} else if ( i != 2 ) {
		printf(error, "Dequeue Returned Wrong Item");
	}

	// now prepend a new item
	if ( -1 == queue_prepend(q, (any_t)(NUM_APPENDS + 1)) ) {
		printf(error, "Prepend Returned Failure Code");
	}

	// try dequeue again (see if prepend works too)
	ret = queue_dequeue(q, (any_t*)&i);
	if ( ret == -1 ) {
		printf(error, "Dequeue Returned Failure Code");
	} else if ( i != NUM_APPENDS + 1 ) {
		printf(error, "Dequeue Returned Wrong Item");
	}

	// try dequeue one last time
	ret = queue_dequeue(q, (any_t*)&i);
	if ( ret == -1 ) {
		printf(error, "Dequeue Returned Failure Code");
	} else if ( i != 3 ) {
		printf(error, "Dequeue Returned Wrong Item");
	}

	// at this point, 7 should still be left,
	// so try deleting it
	ret = queue_delete(q, (any_t)7);
	if ( ret == -1 ) {
		printf(error, "Delete Returned Failure Code For Item That Should Exist");
	}

	// make sure 7 really is gone
	i = 1;
	while ( i < 8 ) {
		// these should be in increasing order,
		// so keep going 'til we find 8
		ret = queue_dequeue(q, (any_t*)&i);
		if ( i == 7 ) {
			printf(error, "7 Still Existed Even Though It Was Deleted");
		}
		if ( ret == -1 ) {
			printf(error, "Dequeue Returned Failure Code");
			break;
		}
	}

	// we just dequeued 8, so NUM_APPENDS - 8 should be left
	ret = queue_length(q);
	if ( ret == -1 ) {
		printf(error, "Length Returned Failure Code");
	} else if ( ret != (NUM_APPENDS - 8) ) {
		printf(error, "Length Is Incorrect");
	}

	// alright, done for now, free the memory
	ret = queue_free(q);
	q = NULL;	// just to make sure I don't accidentally use this later.
				// always set ptr's to NULL directly after freeing them

	if ( ret == -1 ) {
		printf(error, "Free Returned Failure Code");
	}

	// an accident is about to happen
	ret = queue_length(q);
	if ( ret != -1 ) {
		printf("Length Thinks It Used a NULL Pointer as a Queue");
	}

	// don't forget to free the string
	free(error);
	error = NULL;

	// Now print out memory leak report (this just works when
	// running in Debug mode in Visual Studio. output can be
	// found in the Output Pane, not the console window
	_CrtDumpMemoryLeaks();
	
	// Finally keep the Command Window open 'til enter is pressed
	system("pause");

	return 0;
}
