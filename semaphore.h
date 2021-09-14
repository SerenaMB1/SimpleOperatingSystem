#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stddef.h>
#include "mutex.h"
#include "task.h"
#include "os.h"

typedef struct {
	uint32_t permits; 
	OS_mutex_t mutex;
} semaphore_t; 

void semaphoreInit(semaphore_t *semaphore, uint32_t permits);
void semaphoreAquire(semaphore_t *semaphore, uint32_t permits);
void semaphoreRelease(semaphore_t *semaphore, uint32_t permits);

#endif /* SEMAPHORE_H */
