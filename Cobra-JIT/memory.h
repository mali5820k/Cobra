#ifndef kc_memory_h
#define kc_memory_h

#include "common.h"

// Calculates a new capacity given current capacity.
#define GROW_CAPACITY(capacity) \
    ((capacity) < 16 ? 16 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif