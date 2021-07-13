#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

/**
 * Hash table for storing strings gets initialized to be empty.
 * A Hash table is essentiallly a dynamic array.
*/
void initTable(Table* table) {
    table -> count = 0;
    table -> capacity = 0;
    table -> entries = NULL;
}

/**
 * Frees the memory and all allocated entries in the specified Hash table.
*/
void freeTable(Table* table) {
    FREE_ARRAY(Entry, table -> entries, table -> capacity);
    initTable(table);
}