/* 
 * This file provides an interface to the basic primitives that
 * support the threads package, scheduler, and semaphore implementations.
 * These primitives include those for allocating and manipulating stacks
 * and for performing atomic operations. These portable interfaces to
 * code written in assembler enable the threads package to be written
 * in a high-level language.
 * 
 * YOU SHOULD NOT MODIFY THIS FILE.
 */
#ifndef __MACHINEPRIMITIVES_H_
#define __MACHINEPRIMITIVES_H_
#include <windows.h>
#include "defs.h"

/* define data types */
typedef void *stack_pointer_t;
typedef int tas_lock_t;		 /* test-and-set locks.  */
typedef int *arg_t;          /* generic function argument */
typedef int (*proc_t)(arg_t); /* generic function pointer */



/* *************************
 * Time Utility Function *
 ************************* */

/*
 *  Returns the current time in milliseconds
 */
unsigned __int64 currentTimeMillis();


/* *************************
 * Stack Utility Functions *
 ************************* */

/*
 *	Allocate a fresh stack.  Stacks are said to grow "down" (from higher
 *  memory locations towards lower ones) on our version of the x86
 *  architecture.
 *
 *	The bottom of the stack is returned in *stackbase; the top of
 *	the stack is returned in *stacktop.
 *
 *	-----------------
 *	|  stacktop	    |  <- next word pushed here
 *	|               |
 *	|               |
 *	|  stackbase    |  <- bottom of stack.
 *	-----------------
 */
extern void minithread_allocate_stack(stack_pointer_t *stackbase,
				      stack_pointer_t *stacktop);


/*
 *	Frees the stack at stackbase.  The calling thread must not be running
 *  on the stack referenced by stackbase, and after this call no
 *  thread may run on the stack referenced by stackbase.
 */
extern void minithread_free_stack(stack_pointer_t stackbase);


/*
 * 	Initialize the stackframe pointed to by *stacktop so that
 *	the thread running off of *stacktop will invoke:
 *		body_proc(body_arg);
 *		final_proc(final_arg);
 *
 *	The call to final_proc should be used for cleanup, since it is called
 *	when body_proc returns.  final_proc should not return; doing so will
 *	lead to undefined behavior and likely cause your system to crash.
 *
 *	body_proc and final_proc cannot be NULL. Passing invalid
 *      function pointers crashes the system.
 *
 *	This procedure changes the value of *stacktop.
 */
extern void minithread_initialize_stack(stack_pointer_t *stacktop, 
					proc_t body_proc,
					arg_t body_arg,
					proc_t final_proc,
					arg_t final_arg);


/* 
 * Context switch primitive.
 *
 * This call will first save the caller's state (i.e. all of its registers)
 * on the stack.
 *
 * It will then save the stack pointer in the location pointed to by old_stacktop.
 * It will replace the processor's stack pointer with the value pointed to by the
 * new_stacktop.
 *
 * Finally, it will reload the rest of the machine registers that were saved on the
 * new thread's stack previously, and thus resume the new thread from where it left
 * off.
 *
 * old_stacktop is only used to return out the stacktop pointer for the old thread;
 * the dereferenced value is never read from.
 *
 * The first time this is called, old_stacktop will point to the stacktop of of the
 * default stack that the system is originally running on.
 */
extern void minithread_switch(stack_pointer_t *old_stacktop,
			      stack_pointer_t *new_stacktop);



/* ****************************
 * Synchronization Primitives *
 * ************************** */

/*
 *	Atomically test and set the value at l to 1.  Return old value found by test.
 */
extern int atomic_test_and_set(tas_lock_t *l);


/*
 *	Atomically set the value at l to 0.
 */
extern void atomic_clear(tas_lock_t *l);


/* 
 * Atomically set the value pointed to be x to be newval, and return
 * the old value of x.
 */
extern int swap(int* x, int newval);


/*
 * Atomic compare and swap.
 * If the value pointed to by x is equal to oldval, then replace it with
 * newval; regardless of the result of the comparison, return the original
 * value of *x.
 */
extern int compare_and_swap(int* x, int oldval, int newval);


#endif __MACHINEPRIMITIVES_H_
