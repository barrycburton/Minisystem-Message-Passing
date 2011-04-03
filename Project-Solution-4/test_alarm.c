/*
 * This file should contain a single, succint test for the alarm API
 * It should completely test this API (all functions) but should not be used
 * to test other APIs. In this file, assume that all other APIs work as they
 * should. This file should contain no memory leaks so that any memory leaks
 * here will be known to be inside the alarm API. The tests here
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

// Local Includes
#include "defs.h" // this includes the the memory leak detection setup
#include "alarm.h"


// Define utility functions here - there should be at least one utility
// function for each problem category, but there may be more





int main(void) {
	// Do tests here - call utility functions as needed









	// Now print out memory leak report (this just works when
	// running in Debug mode in Visual Studio. output can be
	// found in the Output Pane, not the console window
	_CrtDumpMemoryLeaks();
	
	// Finally keep the Command Window open 'til enter is pressed
	system("pause");

	return 0;
}
