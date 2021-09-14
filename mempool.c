#include "mempool.h"

/* This is an implementation of a Memory Pool
	 
	 The Memory pool implemented as singly linked list. */

/* Initialise the Memory Pool*/
void pool_init(pool_t *pool) {
    // Initialise the pool
		pool->head = NULL;
	  mutex_init(&pool->mutex);
}

/* Allocate from the Memory Pool*/
void *pool_allocate(pool_t *pool) {
    // Return the head of the list of blocks
    // Update the head pointer
		//mutex_aquire(&pool->mutex);
		void* item = pool->head;
		pool->head = *(void**)item;	
	 	//mutex_release(&pool->mutex);

	
	return item;  
}

/* Deallocate to the Memory Pool*/
void pool_deallocate(pool_t *pool, void *item) {
    // Add the new item to the head of the list
		//mutex_aquire(&pool->mutex);
		*(void**) item = pool->head;
		pool->head = item;
		//mutex_release(&pool->mutex);
}
