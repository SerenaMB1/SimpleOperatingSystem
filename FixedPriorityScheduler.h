#ifndef __FixedPriorityScheduler_h__
#define __FixedPriorityScheduler_h__

#include "os.h"

// define max tasks system can process
#define SIMPLE_FP_MAX_TASKS 8

extern OS_Scheduler_t const fixedPriorityScheduler;
static uint32_t isTaskRunnable(OS_TCB_t *task, const uint32_t curTicks);

enum FPSPriority {
	HIGH =16,
	MEDIUM = 8,
	LOW = 4
};

#endif /* __FixedPriorityScheduler_h__ */
