#include "os.h"
#include <stdio.h>
#include <string.h>
#include "utils/config.h"
#include "FixedPriorityScheduler.h"
#include "sleep.h"
#include "mutex.h"
#include "queue.h"
#include "memory.h"

#define packet_MAX_BUFFER 256

/* DEMONSTRATION CODE 

	 This code will demonstrate how:
	 
	 Replacing the round robin scheduler with a fixed priority scheduler which allows tasks to be executed for a given slice of time. 
	 Queue based communication to send messages between tasks.  
	 Recursive mutexs which gives to guarantee that only one task can use a shared resource at a time. Without causing a deadlock itself. 
	 Counting semaphores that introduce a level of protection from problems such as the overflowing/overwriting of data in arrays or queues. 
   Memory Pools protected by a mutexs allow for dynamic and efficient use of memory.  
*/

typedef struct {
	uint32_t id;
	char data[packet_MAX_BUFFER];
} packet_t;

/* List of static variables that */ 
static OS_mutex_t mutexT;
static queue_t printQueue;
static queue_t animalQueue;
static pool_t packetPool;
static packet_t packets[25];
static uint32_t packetID = 0;

/* Create packets */
packet_t* getPacket(){
		packet_t *packet;
		while(1){
			packet = pool_allocate(&packetPool);
			if (!packet) {
				OS_sleep(1);
				continue;
			}
			if(packetID == UINT32_MAX){
				packetID = 0;
			}
			packet->id = packetID++;
			return packet;
	}		
}
/* Sends name chars to queue to be read by the animalsTask function*/
void animalNamesTask(void const *const args) {
	int taskCounter = 0;
	while (1) {
			packet_t *packet = getPacket();
			// Store animal names into packet
			switch(taskCounter) {
				case 1:
					 snprintf(packet->data, packet_MAX_BUFFER, "Tiger");
					break;
				case 3:
					 snprintf(packet->data, packet_MAX_BUFFER, "Mouse");
					break;
				case 5:
					 snprintf(packet->data, packet_MAX_BUFFER, "Elephant");
					break;
				case 7:
					 snprintf(packet->data, packet_MAX_BUFFER, "Snake");
					break;
				case 9:
					 snprintf(packet->data, packet_MAX_BUFFER, "Capybara");
					break;
				default:
					snprintf(packet->data, packet_MAX_BUFFER, "Wind");
			}
			queueSend(&animalQueue, &packet);
			taskCounter = (taskCounter + 1) % 15;	
	}
}

/* Print out animal names from queue to demostrate recieving a message from another task */
void animalsTask(void const *const args) {
	while (1) {
		packet_t *packet = queueReceive(&animalQueue);
		packet_t *animalpacket = getPacket();
		snprintf(animalpacket->data, packet_MAX_BUFFER, "animalsTask: The %s says 'Hello'!", packet->data);
		pool_add(&packetPool, packet);
		queueSend(&printQueue, &animalpacket);
	}
}

/* Print fibonacci numbers from other task */
void printTask(void const *const args) {
	while (1) {
		packet_t* packet = queueReceive(&printQueue);
		printf("> %u: %s\n", packet->id, packet->data);
		pool_add(&packetPool, packet);
	}
}

/* Calculate the Fibonacci sequence, send calculated data to the print task to demonstrate the queue*/
void taskFib(void const *const args) {
	uint32_t previousFib = 1, currentFib = 1, tmpFib = 0, counterFib = 0;
	packet_t* packet;
	while (1) {
		packet = getPacket();
		// Sleep until the next memory block is available
		if(!packet) {
			OS_sleep(10);
			continue;
		}
		// Calculate Fib sequence
		tmpFib = previousFib + currentFib;
		previousFib = currentFib;
		currentFib = tmpFib;
		counterFib++;
		//Max n in Fib sequence
		if(counterFib >= 43){
			previousFib = 1, currentFib =1, tmpFib =0 , counterFib =0;
		}
		// Write data to packet to be sent via the queue
		snprintf(packet->data, packet_MAX_BUFFER, "taskFib: %u", currentFib);
		queueSend(&printQueue, &packet);
		OS_sleep(5);
	}
}

/* On initialisation populate the memory pool ready for use */
void populatePoolTask(void const * const args){
	// Clear data and add to the pool 
	for(int loopCounter = 0; loopCounter < 25; loopCounter++){
		pool_add(&packetPool, &packets[loopCounter]);
	}
}

/* MAIN FUNCTION */

   int main(void) {
	/* Set up core clock and initialise serial port */
	config_init();
	mutexInit(&mutexT); 
	queueInit(&printQueue);
	queueInit(&animalQueue);
	pool_init(&packetPool);

	printf("\r\nDocetOS Sleep and Mutex\r\n");

	/* Reserve memory for five stacks and five TCBs.
	   Remember that stacks must be 8-byte aligned. */
	__align(8)
	static uint32_t stack1[128], stack2[128], stack3[128], stack4[128], stack5[128];
	static OS_TCB_t TCB1, TCB2, TCB3, TCB4, TCB5;

	/* Initialise the TCBs using the two functions above */

	OS_initialiseTCB(&TCB5, stack5+64, populatePoolTask, 0, HIGH);
	OS_initialiseTCB(&TCB1, stack1+64, animalNamesTask, 0, MEDIUM);
	OS_initialiseTCB(&TCB2, stack2+64, animalsTask, 0, MEDIUM);
	OS_initialiseTCB(&TCB3, stack3+64, printTask, 0, HIGH);
	OS_initialiseTCB(&TCB4, stack4+64, taskFib, 0, LOW);

	/* Initialise and start the OS */
	OS_init(&fixedPriorityScheduler);
	OS_addTask(&TCB5);
	OS_addTask(&TCB1);
	OS_addTask(&TCB2);
	OS_addTask(&TCB3);
	OS_addTask(&TCB4);
	OS_start();
}
