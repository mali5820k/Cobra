#ifndef kc_table_h
#define kc_table_h

#include "common.h"
#include "value.h"

/**
 * Entry types include the string object pointer reference and the value of that string
 * for storing into the Hash table in a concise and organized manner.
*/
typedef struct {
    ObjString* key;
    Value value;
} Entry;

/**
 * The Hash table itself needs to function like a dynamic array and 
 * includes counters for current capacity and count of entries, as well
 * as the list of Entries itself.
*/
typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

/**
 * Hash table for storing strings gets initialized to be empty.
 * A Hash table is essentiallly a dynamic array.
*/
void initTable(Table* table);

/**
 * Frees the memory and all allocated entries in the specified Hash table.
*/
void freeTable(Table* table);


#endif