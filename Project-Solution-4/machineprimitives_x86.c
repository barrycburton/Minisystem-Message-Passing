/*
 * Minithreads x86/NT Machine Dependent Code
 *
 * You should NOT modify this file.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>     // included for currentTimeMillis
#include <sys/timeb.h>

#include "defs.h"
#include "machineprimitives.h"

// Remove this after interrupts are included
typedef int interrupt_level_t;
interrupt_level_t interrupt_level;

// Pragma to avoid warnings from modifying registers
#pragma warning(disable:4731)

unsigned __int64 currentTimeMillis() {  
  struct _timeb timebuffer;
  unsigned __int64 lt = 0;
  _ftime_s(&timebuffer);
  lt = timebuffer.time;
  lt = lt*1000;
  lt = lt+timebuffer.millitm;
  return lt;
}

/* atomic_test_and_set - using the native compare and exchange on the 
   Intel x86; returns 0 if we set, 1 if not (think: l == 1 => locked,
   and we return the old value, so we get 0 if we managed to lock l).
*/

int atomic_test_and_set(tas_lock_t *l) {
  int val;
  
  _asm
    {
      mov edx, dword ptr [l]  ; Get the pointer to l
				  
      mov ecx, 1              ; load 1 into the cmpxchg source
      mov eax, 0              ; load 0 into the accumulator

			                 ; if l == 0 then
      lock cmpxchg dword ptr [edx], ecx  ;     l = 1 (and eax = 0)
		                         ; else
					 ;     (l = 1 and) eax = 1
									  
      mov val, eax            ; set the accumulator to be the return val
    }

  return val;
}

/*
 * swap
 * 
 * atomically stores newval in *x, returns old value in *x
 */
int swap(int* x, int newval) {
  int retval;

  _asm {
    mov eax, newval
    mov edx, dword ptr [x]

    lock xchg dword ptr [edx], eax
    mov retval, eax
  }

  return retval;

}

/*
 * compare and swap
 * 
 * compare the value at *x to oldval, swap with
 * newval if successful
 */
int compare_and_swap(int* x, int oldval, int newval) {
  int retval;

  _asm {
    mov eax, oldval
    mov ecx, newval
    mov edx, dword ptr [x]

    lock cmpxchg dword ptr [edx], ecx
    mov retval, eax
  }

  return retval;
}

/*
 * atomic_clear
 *
 */

void atomic_clear(tas_lock_t *l) {
	*l = 0;	
}


/*
 * minithread_root
 *
 */
int minithread_root() {
  _asm
    {
      pop edi      ; compiler tries to save some state that we
      pop esi      ; do not want it to.
      pop ebx      ;
      mov esp, ebp ;
      pop ebp      ;

      push edi    ; push the arg to the main proc
      call ebx    ; call main proc

      push ebp    ; push the arg to the clean-up proc
      call esi    ; call the clean-up

      ; Should never get here
    }

  kprintf("An error has occurred, this statement should not be reached.");

}

/*
 * minithread_switch - on the intel x86
 *
 */
void minithread_switch(stack_pointer_t *old_stacktop,
		       stack_pointer_t *new_stacktop) {
  _asm
    {

      mov ecx, old_stacktop  ; Get these before we clobber
      mov eax, dword ptr [new_stacktop]  ; the ebp register.

      pop edi	   ; compiler tries to save some state which we
      pop esi      ; don't want it to.
      pop ebx      ;
      mov esp, ebp ; Brings sp to where the old bp is stored. 
      pop ebp      ;

      push ebp	   ; Save the ebp, esi, edi, and ebx on the stack
      push esi     ;
      push edi     ;
      push ebx     ;
		        
      mov dword ptr [ecx], esp  ; pass back the old thead's sp

      mov esp, dword ptr [eax]  ; deref. the pointer and load new thread's sp

      mov interrupt_level, 1 ; re-enable interrupts

      pop ebx		; Get the ebp, esi, edi, and ebx off the stack
      pop edi		;
      pop esi		;
      pop ebp		;

      ret 0;
    }
}
