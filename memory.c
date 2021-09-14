#include "memory.h"
/* This is an implementation of a Memory Pool
	 
	 The Memory pool implemented as singly linked list.
	 It is protected with a mutex which only allows one 
	 task to write to the pool at a time
*/

/* Initialise the Memory Pool*/
void pool_init(pool_t *pool) {
    // Initialise the pool
		pool->head = NULL;
	  mutexInit(&pool->mutex);
}

/* Allocate from the Memory Pool*/
void *pool_allocate(pool_t *pool) {
    // Return the head of the list of blocks
    // Update the head pointer
		mutexAquire(&pool->mutex);
		void* item = pool->head;
		pool->head = *(void**)item;	
	 	mutexRelease(&pool->mutex);
		return item;  
}

/* Deallocate to the Memory Pool*/
void pool_deallocate(pool_t *pool, void *item) {
    // Add the new item to the head of the list
		mutexAquire(&pool->mutex);
		*(void**) item = pool->head;
		pool->head = item;
		mutexRelease(&pool->mutex);
}
