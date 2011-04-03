/*
 * This file should contain a single, succint test for the directory API
 * It should completely test this API (all functions) but should not be used
 * to test other APIs. In this file, assume that all other APIs work as they
 * should. This file should contain no memory leaks so that any memory leaks
 * here will be known to be inside the directory API. The tests here
 * should be divided into groups corresponding to the Problem Categories
 * from your Design Document (don't forget to include the corner cases in
 * group for the category they fit into.)
 *
 * Goal: assuming that all libraries which this one depends on are working
 * correctly, after running this we should be convinced that this library
 * works in the most common use cases and avoids the most common bugs.
 *
 * Remember: keep it SHORT and ORGANIZED. if long or unorganized, it is
 * very hard to look at the testing code and be convinced that it actually
 * accomplishes the above goal.
 */


// Platform Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Local Include
#include "defs.h" // this includes the the memory leak detection setup

// Test Include
#include "directory.h"


// Utility Function Declarations
// there should be a utilty function
// for each problem category





int main(void) {
	// Do setup if needed


	// Testing Problem Category X


	// Testing Problem Category Y

	
	// ...



	dbgprintf("Memory Leaks (If Any) Follow:\n");
	_CrtDumpMemoryLeaks();
	system("pause");
	return 0;
}
