#include "simpleRoundRobin.h"
#ifndef __STM32F3xx_H
#include "stm32f3xx.h"
#endif
/* This is an implementation of an extremely simple (and inefficient!) round-robin scheduler.

   An array of pointers to TCBs is declared, and when tasks are added they are inserted into
	 this array.  When tasks finish, they are removed from the array (pointers are set back to
	 zero).  When the scheduler is invoked, it simply looks for the next available non-zero
	 pointer in the array, and returns it.  If there are no non-zero TCB pointers in the array,
	 a pointer to the idle task is returned instead.
	 
	 The inefficiency in this implementation arises mainly from the way arrays are used for list
	 storage.  If the array is large and the number of tasks is small, the scheduler will spend
	 a lot of time looking through the array each time it is called. */

/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const * simpleRoundRobin_scheduler(void);
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb);
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb);
static void simpleRoundRobin_wait(void* const reason, uint32_t checkCode);
static void simpleRoundRobin_notify(void* const reason);

static OS_TCB_t * tasks[SIMPLE_RR_MAX_TASKS] = {0};

/* Scheduler block for the simple round-robin */
OS_Scheduler_t const simpleRoundRobinScheduler = {
	.preemptive = 1,
	.scheduler_callback = simpleRoundRobin_scheduler,
	.addtask_callback = simpleRoundRobin_addTask,
	.taskexit_callback = simpleRoundRobin_taskExit,
	.wait_callback = simpleRoundRobin_wait,
	.notify_callback = simpleRoundRobin_notify 
};


/* Round-robin scheduler callback */
static OS_TCB_t const * simpleRoundRobin_scheduler(void) {
	static int i = 0;
	// Clear the yield flag if it's set - we simply don't care
	OS_currentTCB()->state &= ~TASK_STATE_YIELD;
	for (int j = 1; j <= SIMPLE_RR_MAX_TASKS; j++) {
		i = (i + 1) % SIMPLE_RR_MAX_TASKS;
		if (tasks[i] != 0 && !(tasks[i]->state & TASK_STATE_WAIT)) {
			//skip if in a wait state 
		
			if (tasks[i]->state & TASK_STATE_SLEEP) {
				//check if the right amount of time has elapsed 
				if (tasks[i]->data < OS_elapsedTicks()){
						tasks[i]->state &= ~TASK_STATE_SLEEP;
						return tasks[i];
				}	
			}
			else {
				return tasks[i];
			}
		}
	}
	// No tasks in the list, so return the idle task
	return OS_idleTCB_p;
}

/* 'Add task' callback */
static void simpleRoundRobin_addTask(OS_TCB_t * const tcb) {
	for (int i = 0; i < SIMPLE_RR_MAX_TASKS; i++) {
		if (tasks[i] == 0) {
			tasks[i] = tcb;
			return;
		}
	}
	// If we get here, there are no free TCB slots, so the task just won't be added
}

/* 'Task exit' callback */
static void simpleRoundRobin_taskExit(OS_TCB_t * const tcb) {
	// Remove the given TCB from the list of tasks so it won't be run again
	for (int i = 0; i < SIMPLE_RR_MAX_TASKS; i++) {
		if (tasks[i] == tcb) {
			tasks[i] = 0;
		}
	}	
}

static void simpleRoundRobin_wait(void* const reason, uint32_t checkCode){
//store the reason code in the current TCB by using the data field
	//check code 

	if(checkCode == currentCheckValue()){
		OS_currentTCB()->data = (uint32_t)reason; 
		OS_currentTCB()->state |= TASK_STATE_WAIT; //Waiting state 
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	}
	else {
		return; 
	}
	
}
static void simpleRoundRobin_notify(void* const reason){
		for (int i = 0; i < SIMPLE_RR_MAX_TASKS; i++) {
		if (tasks[i] != 0) {
			if ((tasks[i]->state & TASK_STATE_WAIT) && (tasks[i]->data == (uint32_t)reason)) {
					tasks[i]->state &= ~TASK_STATE_WAIT;
					tasks[i]->data = 0;
				}
		}
	}
}
