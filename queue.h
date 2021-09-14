#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include "mutex.h"
#include "task.h"
#include "os.h"
#include "semaphore.h"

#define MAX_QUEUE_SIZE 10
	
typedef struct {
	OS_mutex_t mutex;
	semaphore_t semaphore;
	void *data[MAX_QUEUE_SIZE];
	int insert;
	int retrieve;
} queue_t;


void queueInit(queue_t *queue);
void *queueReceive(queue_t *queue);
void queueSend(queue_t *queue, void* data);

#endif /* QUEUE_H */
