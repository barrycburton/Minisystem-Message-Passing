/* Directory Implementation
 * this data structure holds (key,value) mappings
 * and allows the efficient retreival of a value
 * from its key
 */


#include "directory.h"
#include "defs.h"


/* data types for directory
 */

#define RESIZE_RATIO_NUMER (3)
#define RESIZE_RATIO_DENOM (4)

#define SIZE_INITIAL (128)


typedef struct entry* entry_t;
struct entry {
	key_t key;
	any_t value;
	entry_t next;
};

struct directory {
	entry_t *table;
	int table_size;
	int entry_count;
};


typedef enum dir_retrieve dir_retrieve_t;
enum dir_retrieve {
	DIR_RETRIEVE,
	DIR_RETRIEVE_DELETE
};



/*
 * utility function declarations
 */
int directory_table_resize(directory_t obj);
int directory_entry_insert(directory_t obj, entry_t item);
int directory_entry_retrieve(directory_t obj, key_t key, any_t *value_p, dir_retrieve_t action);
entry_t *directory_entry_find_slot(directory_t obj, key_t key);




/* Create a new directory data structure which should be initialized
 * and will contain no entries.
 * On success return a pointer to the new directory.
 * On failure return NULL.
 */
directory_t directory_new() {
	directory_t obj = malloc(sizeof(struct directory));
	obj->entry_count = 0;
	obj->table_size = SIZE_INITIAL;
	obj->table = calloc(SIZE_INITIAL, sizeof(entry_t));
	return obj;
}


/* Destory a directory and all memory used by it.
 * On success return 0
 * On failure return -1
 */
int directory_destroy(directory_t obj) {
	if ( obj ) {
		entry_t temp;
		int i;
		for (i=0; i < obj->table_size; i++) {
			while ( obj->table[i] ) {
				temp = obj->table[i];
				obj->table[i] = obj->table[i]->next;
				free(temp);
			}
		}
		free(obj->table);
		free(obj);
		return 0;
	}
	return -1;
}


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
int directory_iterate(directory_t obj, PFmap iter_func, key_t key, any_t item) {
	if ( obj && iter_func ) {
		entry_t temp;
		int i;
		int ret = 0;
		for (i=0; i < obj->table_size; i++) {
			temp = obj->table[i];
			while ( temp && ret != -1 ) {
				ret = (*iter_func)(temp->key, temp->value, key, item);
				temp = temp->next;
			}
			if ( -1 == ret ) {
				break;
			}
		}
		return ret;
	}
	return -1;
}


/*
 * return the number of key/value mappings in the
 * directory, or -1 (failure).
 */
int directory_size(directory_t obj) {
	return obj->entry_count;
}


/*
 * add the given key / value mapping to the directory.
 * note that keys and values may take on any value.
 * return -1 (failure) or 0 (success).
 */
int directory_add(directory_t obj, key_t key, any_t item) {
	if ( obj ) {
		entry_t new_item;

		// Check for resize
		if ( (obj->table_size * RESIZE_RATIO_NUMER) < ( obj->entry_count * RESIZE_RATIO_DENOM) ) {
			directory_table_resize(obj);
		}

		// Build Entry
		new_item = malloc(sizeof(struct entry));
		new_item->key = key;
		new_item->value = item;
		new_item->next = NULL;

		// Insert
		return directory_entry_insert(obj, new_item);
	}
	return -1;
}


/*
 * return the value associated with key.
 * if value_p is null or key is not found,
 * return -1 (failure).
 * otherwise return 0 (success).
 */
int directory_get(directory_t obj, key_t key, any_t *value_p) {
	if ( obj && value_p ) {
		return directory_entry_retrieve(obj, key, value_p, DIR_RETRIEVE);
	}
	return -1;
}


/*
 * remove the value associated with key.
 * if value_p is non-null, return value out through it.
 * if key is not found, return -1 (failure)
 * if key is found, return 0 (success)
 */
int directory_remove(directory_t obj, key_t key, any_t *value_p) {
	int ret = -1;
	if ( obj ) {
		any_t temp = NULL;
		ret = directory_entry_retrieve(obj, key, &temp, DIR_RETRIEVE_DELETE);
		if (value_p && -1 != ret) {
			*value_p = temp;
		}
	}

	return ret;
}




/*
 * Utility Function Implementations
 */


int directory_table_resize(directory_t obj) {
	entry_t *old_table = obj->table;
	int old_size = obj->table_size;
	entry_t item;
	int i;
	obj->table_size *= 2;
	obj->table = malloc(obj->table_size * sizeof(entry_t));
	for (i=0; i < old_size; i++ ) {
		while ( old_table[i] ) {
			item = old_table[i];
			old_table[i] = item->next;
			directory_entry_insert(obj, item);
		}
	}
	free(old_table);

	return 0;
}


int directory_entry_insert(directory_t obj, entry_t item) {
	// get location
	entry_t *location_p = directory_entry_find_slot(obj, item->key);

	if ( *location_p ) {
		// update existing
		item->next = (*location_p)->next;
		free(*location_p);
		*location_p = item;
	} else {
		// just insert new
		*location_p = item;
		obj->entry_count++;
	}

	return 0;
}


int directory_entry_retrieve(directory_t obj, key_t key, any_t *value_p, dir_retrieve_t action) {
	// get location
	entry_t *location_p = directory_entry_find_slot(obj, key);

	// does it exist?
	if ( *location_p ) {
		*value_p = (*location_p)->value;

		// should we delete?
		if ( DIR_RETRIEVE_DELETE == action ) {
			entry_t item = *location_p;
			*location_p = item->next;
			free(item);
			obj->entry_count--;
		}

		return 0;
	}

	return -1;
}


entry_t* directory_entry_find_slot(directory_t obj, key_t key) {
	// find table location
	entry_t *location_p = &(obj->table[(unsigned int)key % obj->table_size]);

	// now find first link in chain that is empty or matches the key
	while ( *location_p && (*location_p)->key != key ) {
		location_p = &((*location_p)->next);
	}
	
	return location_p;
}
