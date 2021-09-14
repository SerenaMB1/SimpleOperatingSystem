#include "mutex.h"

/* This is an implementation of Recursive Mutual Exclusion 
	 (Recursive Mutex)
	 
	 Tasks can recursively acquire this mutex without a deadlock 
	 as the mutext makes note of which task aquires the mutex and 
	 how many times it is used.
	 
	 The Mutex can be used to protect tasks. */

/* Initialise the mutex to avoid garbage data*/
void mutexInit(OS_mutex_t * mutex){
	mutex->counter = 0;
	mutex->task = 0;
}

/* Aquire the mutex */
void mutexAquire(OS_mutex_t * mutex){
	uint32_t currentTCB;
	while (1) {
		// Wrapper for assembly code to ensure processor doesn't interrupt task. Exclusive Load.
		currentTCB = __LDREXW((uint32_t *) &(mutex->task));
		if (currentTCB == 0) {
			// Wrapper for assembly code to ensure processor doesn't interrupt task. Exclusive Store.
		  if (__STREXW((uint32_t) OS_currentTCB(), (uint32_t *) &(mutex->task)) == 0){
				break;
			}
			// Put task into wait state if mutex isn't acquired 
		} else if (currentTCB != (uint32_t) OS_currentTCB) {
			OS_wait((void *) mutex, currentCheckValue());
		}
	}
	// Else increase the mutex count - how many times this task has taken the mutex
	mutex->counter++;
}

/* Release the Mutex*/
void mutexRelease(OS_mutex_t * mutex){
	// Check if current task is equal to mutex task
	if(OS_currentTCB() == mutex->task){
		mutex->counter--;
			if(mutex->counter == 0){
				//mutex has been released
				mutex->task = 0;
				// Notify tasks that they can now claim the mutex 
				OS_notify((void *)mutex);
			}
	}
}
