#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include "mutex.h"
#include "task.h"
#include "os.h"

typedef struct {
	void *head;
	OS_mutex_t mutex;
} pool_t;

void pool_init(pool_t *pool);
void *pool_allocate(pool_t *pool);
void pool_deallocate(pool_t *pool, void *item);

#define pool_add pool_deallocate

#endif /* MEMORY_H */
