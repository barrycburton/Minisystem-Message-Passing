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

directory_t dir = directory_new();
any_t value;
int key;

printf("%d\n", directory_size(dir));

directory_add(dir, 5, "five");
directory_add(dir, 10, "ten");
directory_add(dir, 15, "fifteen");
directory_add(dir, 0, "zero");

printf("%d\n", directory_size(dir));

key = 10;
directory_get(dir, key, &value);
printf("%d == %s?\n", key, value);

key = 5;
directory_remove(dir, key, &value);
printf("%d == %s?\n", key, value);

key = 5;
value = "not found";
directory_remove(dir, key, &value);
printf("%d == %s?\n", key, value);

directory_add(dir, 128, "onehundredtwentyeight");

key = 0;
directory_get(dir, key, &value);
printf("%d == %s?\n", key, value);

key = 128;
directory_get(dir, key, &value);
printf("%d == %s?\n", key, value);

directory_add(dir, 0, "zero has been updated");
directory_add(dir, 10, "ten has been updated");

key = 0;
directory_get(dir, key, &value);
printf("%d == %s?\n", key, value);

key = 10;
directory_get(dir, key, &value);
printf("%d == %s?\n", key, value);

key = 10;
directory_remove(dir, key, NULL);
value = "not found";
directory_get(dir, key, &value);
printf("%d == %s?\n", key, value);

directory_destroy(dir);

	dbgprintf("Memory Leaks (If Any) Follow:\n");
	_CrtDumpMemoryLeaks();
	system("pause");
	return 0;
}
