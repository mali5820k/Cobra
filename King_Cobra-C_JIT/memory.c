#include <stdlib.h>
#include "memory.h"

// Reallocates an array to a specific memory location
// with the new size of the array taken into account.
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if(newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if(result == NULL) exit(1);

    return result;
}
