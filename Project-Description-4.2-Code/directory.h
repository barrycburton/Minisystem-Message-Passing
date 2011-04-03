#ifndef __DIRECTORY_H__
#define __DIRECTORY_H__


/* Directory Interface
 * this data structure holds (key,value) mappings
 * and allows the efficient retreival of a value
 * from its key
 */

/* data type for keys - just an int
 */
typedef int key_t;

/* data type for values - any_t can hold anything
 */
typedef void *any_t;

/* function pointer type for directory iteration function
 */
typedef int (*PFmap)(key_t, any_t, key_t, any_t);

/* data type for directory
 */
typedef struct directory *directory_t;


/* Create a new directory data structure which should be initialized
 * and will contain no entries.
 * On success return a pointer to the new directory.
 * On failure return NULL.
 */
extern directory_t directory_new();


/* Destory a directory and all memory used by it.
 * On success return 0
 * On failure return -1
 */
extern int directory_destroy(directory_t obj);


/* If any parameters are invalid or the directory is empty,
 * return -1 (failure).
 * Otherwise, iterate through the directory, calling the iter_func function
 * on each item in the queue. (The current key/value pair is passed
 * as the first two paramters to iter_func, and the key/value pair passed as
 * as an argument to this function are passed as the third and fourth
 * parameters to iter_func. If at any point iter_func returns -1,
 * then stop iterating and return -1 (failure).
 * Otherwise, return 0 (success).
 */
extern int directory_iterate(directory_t obj, PFmap iter_func, key_t key, any_t item);


/*
 * return the number of key/value mappings in the
 * directory, or -1 (failure).
 */
extern int directory_size(directory_t obj);


/*
 * add the given key / value mapping to the directory.
 * note that keys and values may take on any value.
 * return -1 (failure) or 0 (success).
 */
extern int directory_add(directory_t obj, key_t key, any_t item);


/*
 * return the value associated with key.
 * if value_p is null or key is not found,
 * return -1 (failure).
 * otherwise return 0 (success).
 */
extern int directory_get(directory_t obj, key_t key, any_t *value_p);


/*
 * remove the value associated with key.
 * if value_p is non-null, return value out through it.
 * if key is not found, return -1 (failure)
 * if key is found, return 0 (success)
 */
extern int directory_remove(directory_t obj, key_t key, any_t *value_p);


#endif __DIRECTORY_H__
