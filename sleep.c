#include "sleep.h"
#include "os_internal.h"


//Store the time that the task has asked to be woken into its TCB 
void OS_sleep(uint32_t sleepTime){
	OS_currentTCB()->data = OS_elapsedTicks() + sleepTime; //Rerturns a pointer to the running tasks TCB
	OS_currentTCB()->state = TASK_STATE_SLEEP;
	OS_yield(); 
}
