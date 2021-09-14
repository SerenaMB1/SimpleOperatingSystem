#include "FixedPriorityScheduler.h"
#ifndef __STM32F3xx_H
#include "stm32f3xx.h"
#include <stdio.h>
#endif
/* This is an implementation of a Fixed-Priority Scheduler.

   An array of pointers to TCBs is declared, and when tasks are added they are inserted into
	 this array.  When tasks finish, they are removed from the array (pointers are set back to
	 zero). 
	 
	 When the scheduler is invoked, the program checks whether there is a runnable task. If a 
	 runnable taske exists, the priority is checked and the task is run. 
	 
	 The inefficiency in this implementation arises mainly from the way arrays are used for list
	 storage.  If the array is large and the number of tasks is small, the scheduler will spend
	 a lot of time looking through the array each time it is called. */

/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const *fixedPriorityScheduler_scheduler(void);
static void fixedPriorityScheduler_addTask(OS_TCB_t * const tcb);
static void fixedPriorityScheduler_taskExit(OS_TCB_t * const tcb);
static void fixedPriorityScheduler_wait(void* const reason, uint32_t checkCode);
static void fixedPriorityScheduler_notify(void* const reason);

/* Define maximum tasks */
static OS_TCB_t * tasks[SIMPLE_FP_MAX_TASKS] = {0};


/* Scheduler block for the Fixed-Priority Scheduler */
OS_Scheduler_t const fixedPriorityScheduler = {
	.preemptive = 1,
	.scheduler_callback = fixedPriorityScheduler_scheduler,
	.addtask_callback = fixedPriorityScheduler_addTask,
	.taskexit_callback = fixedPriorityScheduler_taskExit,
	.wait_callback = fixedPriorityScheduler_wait,
	.notify_callback = fixedPriorityScheduler_notify 
};

//TODO: Check OS_TCB_t data field as it is being used for reason in Notify / Wait As well as tick counter in Scheduler


/* Fixed-Priority Scheduler callback */
static OS_TCB_t const *fixedPriorityScheduler_scheduler(void) {
	static int i = 0;
	// store the elapsed ticks value at the start of the task
	const uint32_t OSticks = OS_elapsedTicks();
	const OS_TCB_t *OSCurrentTask = OS_currentTCB();
	for (int j = 0; j < SIMPLE_FP_MAX_TASKS; j++) {
		// Determine whether there is a current task and if it is runnable
		if (tasks[i] == NULL || !isTaskRunnable(tasks[i], OSticks)) {
			// Consider next task 
			i = (i + 1) % SIMPLE_FP_MAX_TASKS; 
			continue; 
		}	
		// Check if the current task have ticks left?
		if (tasks[i]->ticks > OSticks) {
			// If yes, return task
			return tasks[i]; 
		}
		else {
			// If no, check if task is currently running
			if (tasks[i] == OSCurrentTask){
				// if yes, this task is out of time. Consider the next task
				i = (i + 1) % SIMPLE_FP_MAX_TASKS;
				continue;
			}
			else {
				// If no, task needs to be run again. Set its new tick value depending on priority
				if (tasks[i]->priority == HIGH) {
					tasks[i]->ticks = OSticks + HIGH;
				}
				else if (tasks[i]->priority == MEDIUM) {
					tasks[i]->ticks = OSticks + MEDIUM;
				}		
				else {
					tasks[i]->ticks = OSticks + LOW;
				}
			}
		}
		return tasks[i];					
	}
	// If there are no valid tasks in the task list, return the idle task. 
	return OS_idleTCB_p;
}

/*Fixed-Priority Scheduler function to determine whether a task is runnable by checking the state*/
static uint32_t isTaskRunnable(OS_TCB_t *task, const uint32_t curTicks) {
	// Check if task is in yield state
	if (task->state & TASK_STATE_YIELD) {
		// Clear yield state
		task->state &= ~TASK_STATE_YIELD;
		return 0;
	}
	// Check if tasks is in sleep state
	else if (task->state & TASK_STATE_SLEEP) {		
		// Check if the right amount of time has elapsed 
		if (task->data < curTicks) {
			// Clear sleep state
			task->state &= ~TASK_STATE_SLEEP;
			return 1;
		}
		return 0;
	}
	// If task state is wait return task is not runnable
	else if (task->state & TASK_STATE_WAIT) {
		return 0;
	}
	return 1; 
}

/* Add task callback */
static void fixedPriorityScheduler_addTask(OS_TCB_t * const tcb) {
	for (int i = 0; i < SIMPLE_FP_MAX_TASKS; i++) {
		if (tasks[i] == 0) {
			tasks[i] = tcb;
			tasks[i]->data = OS_elapsedTicks();
			return;
		}
	}
	// If we get here, there are no free TCB slots, the task won't be added
}

/* Task exit callback */
static void fixedPriorityScheduler_taskExit(OS_TCB_t * const tcb) {
	// Remove the given TCB from the list of tasks so it won't be run again
	for (int i = 0; i < SIMPLE_FP_MAX_TASKS; i++) {
		if (tasks[i] == tcb) {
			tasks[i] = 0;
		}
	}	
}

/* Task wait callback */
static void fixedPriorityScheduler_wait(void* const reason, uint32_t checkCode){
	// Store the reason code in the current TCB by using the data field
	// Check code increases when notify is called, to allow the task that is about to wait whether it needs to.
	if(checkCode == currentCheckValue()){
		OS_currentTCB()->data = (uint32_t)reason; 
		// Waiting state 
		OS_currentTCB()->state |= TASK_STATE_WAIT; 
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	}
	else {
		return; 
	}
}

/* Task notify callback */
static void fixedPriorityScheduler_notify(void* const reason){
		for (int i = 0; i < SIMPLE_FP_MAX_TASKS; i++) {
		if (tasks[i] != 0) {
			// If task state is wait and data is equal to the reason code 
			if ((tasks[i]->state & TASK_STATE_WAIT) && (tasks[i]->data == (uint32_t)reason)) {
					// Clear wait state
					tasks[i]->state &= ~TASK_STATE_WAIT;
					tasks[i]->data = 0;
			}
		}
		}
}
