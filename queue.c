#include "queue.h"

#ifndef __STM32F3xx_H
#include "stm32f3xx.h"
#include <stdio.h>
#endif

/* This is an implementation of a Pointer Queue-Based Task
	 communication system.
	 
	 The Queue uses a circular buffer to store pointers to 
	 items in memory. 
	 
	 The queue is protected with a semaphore and a mutex. */

/* Initialise the Queue */
void queueInit(queue_t *queue){
	mutexInit(&queue->mutex);
	semaphoreInit(&queue->semaphore, MAX_QUEUE_SIZE);
	queue->insert=0;
	queue->retrieve=0;
	// Null all points int eh queue to protect from garbage
	for(int i=0; i < MAX_QUEUE_SIZE; i++){
		queue->data[i] = NULL;
	}
}

/* Send from the Queue */
void queueSend(queue_t *queue, void* dataPtr) {
	// Wait for space in the queue
	semaphoreAquire(&queue->semaphore, 1);
	void *dp = *(void **)dataPtr;
	// Update the queue atomitcally
	mutexAquire(&queue->mutex);
	queue->data[queue->insert] = dp; 
	queue->insert = (queue->insert + 1) % MAX_QUEUE_SIZE;
	// Notify any readers that we have data
	OS_notify((void *)queue);
	mutexRelease(&queue->mutex);	
}

/* Receive to the Queue*/
void *queueReceive(queue_t *queue) {
	// Check if there is data available
	while(queue->insert == queue->retrieve){
		OS_wait((void *)queue, currentCheckValue());
	}
	// Update the queue atomically
	mutexAquire(&queue->mutex);
	void* tmp = queue->data[queue->retrieve];
	queue->data[queue->retrieve] = NULL;
	// increase size based on max queue size. Prevents overflow
	queue->retrieve = (queue->retrieve + 1) % MAX_QUEUE_SIZE;
	semaphoreRelease(&queue->semaphore, 1);
	mutexRelease(&queue->mutex);
	return tmp;
}
