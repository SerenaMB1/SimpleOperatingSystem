#include "semaphore.h"
#ifndef __STM32F3xx_H
#include "stm32f3xx.h"
#include <stdio.h>
#endif

/* This is an implementation of a Counting Semaphore.
	 
	 The semaphore initialises a set of permits which 
	 are protected by a acquiring a mutex. */

/* Initialise the Semaphore*/
void semaphoreInit(semaphore_t *semaphore, uint32_t permits) {
	semaphore->permits = permits;
	mutexInit(&semaphore->mutex);
}

/* Aquire the Semaphore*/
void semaphoreAquire(semaphore_t *semaphore, uint32_t permits){
	while(1){
		// Check if there are enough permits for this acquire
		if (semaphore->permits - permits <= 0){
		// If there aren't any permits. Task will wait until a permit becomes avaliable
			OS_wait((void *) semaphore, currentCheckValue());
		}else {
		// If there are enough permits. Remove permit and return
			mutexAquire(&semaphore->mutex);
			semaphore->permits -= permits;
			mutexRelease(&semaphore->mutex);
			break;
		}	
	}
}

/* Release the Semaphore*/
void semaphoreRelease(semaphore_t *semaphore, uint32_t permits){
	mutexAquire(&semaphore->mutex);
	// Add a permit back to the semaphore
	semaphore->permits += permits;
	mutexRelease(&semaphore->mutex);
	// Notify any waiting tasks that there are permits available
	OS_notify((void *) semaphore);
}
