#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>
#include <stddef.h>
#include "task.h"
#include "os.h"
//#ifndef __STM32F3xx_H
#include "stm32f3xx.h"
//#endif

typedef struct {
	uint32_t counter;
	OS_TCB_t * task;
} OS_mutex_t;

void mutexInit(OS_mutex_t * mutex);
void mutexAquire(OS_mutex_t * mutex);
void mutexRelease(OS_mutex_t * mutex);

#endif /* _MUTEX_H_ */
