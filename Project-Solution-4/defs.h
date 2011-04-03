#ifndef _DEFS_H_
#define _DEFS_H_

/* #define WINCE */



/* if debuging is desired set value to 1 */
#define DEBUG 0

/* for now kernel printfs are just regular printfs */
#define kprintf printf

#ifdef WINCE
/* Windows CE definitions */
#include <stdio.h>
#include <windows.h>

#define assert(x) \
      if((x) == 0) \
           printf("assert error %s:%d %d\n", __FILE__, __LINE__, GetLastError());
#define time(x) 75489273
#define perror(x) kprintf("There was some error on %s\n",x)

#define SwitchToThread()	Sleep(0)

#define WaitOnObject(mutex)\
  if (WaitForSingleObject(mutex, INFINITE) != WAIT_OBJECT_0) {\
    printf("Error: code %ld.\n", GetLastError());\
    MessageBox(NULL, NULL, NULL, MB_OK| MB_ICONINFORMATION );\
    exit(1);\
  }

#define AbortOnCondition(cond,message) \
 if (cond) {\
    printf("Abort: %s:%d %d, MSG:%s\n", __FILE__, __LINE__, GetLastError(), message);\
    MessageBox(NULL, NULL, NULL, MB_OK| MB_ICONINFORMATION );\
    exit(1);\
 }

#define AbortOnError(fctcall) \
   if (fctcall == 0) {\
    printf("Error: file %s line %d: code %ld.\n", __FILE__, __LINE__, GetLastError());\
    MessageBox(NULL, NULL, NULL, MB_OK| MB_ICONINFORMATION );\
    exit(1);\
  }

#else /* Windows NT definitions */
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <fcntl.h>

// Platform Includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// Set Up Memory Leak Debugging
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Debug Printf (output goes to VS Output window)
#define dbgprintf(message,...) { char *szString = malloc(512); \
	if ( szString ) { \
	sprintf_s(szString, 512, message,__VA_ARGS__); \
	OutputDebugString(szString); \
	free(szString); } }

// Debug Printf with Location (output goes to VS Output window,
//  includes line number and file name)
#define dbglocprintf(message,...) { char *szString = malloc(512); \
	if ( szString ) { \
	sprintf_s(szString, 512, "file: %s, line: %d\t",__FILE__,__LINE__); \
	OutputDebugString(szString); \
	sprintf_s(szString, 512, message,__VA_ARGS__); \
	OutputDebugString(szString); \
	free(szString); } }


/* Macro to clean up the code for waiting on mutexes */
#define WaitOnObject(mutex)\
  if (WaitForSingleObject(mutex, INFINITE) != WAIT_OBJECT_0) {\
      printf("Error: code %ld.\n", GetLastError());\
      exit(1);\
  }

#define AbortOnCondition(cond,message) \
 if (cond) {\
    printf("Abort: %s:%d %d, MSG:%s\n", __FILE__, __LINE__, GetLastError(), message);\
    exit(1);\
 }

#define AbortOnError(fctcall) \
   if (fctcall == 0) {\
      printf("Error: file %s line %d: code %ld.\n", __FILE__, __LINE__, GetLastError());\
      exit(1);\
   }


#endif

#endif /* _DEFS_H_ */
