#ifndef kc_memory_h
#define kc_memory_h

#include "common.h"
#include "object.h"


/**
 * A Utility macro for reallocating memory for various objects.
*/
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

/**
 * Wrapper function that reallocates to 0 bytes.
*/
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

/**
 * Calculates a new capacity given current capacity.
*/
#define GROW_CAPACITY(capacity) \
    ((capacity) < 16 ? 16 : (capacity) * 2)

/**
 * Resizes an array dynamically from current length to new length.
*/
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

/**
 * Frees the memory on the heap that an array occupies.
*/
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/** 
 * Reallocates an array to a specific memory location
 * with the new size of the array taken into account.
*/
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

void markObject(Obj* object);

void markValue(Value value);

void collectGarbage();

/**
 * Parsing through a linked list and freeing all object nodes.
*/
void freeObjects();

#endif