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

/**
 * Get the value associated with the provided key and sets the value variable
 * that's passed in to point to that value. Finally, the function returns true
 * if the value is found, otherwise false.
*/
bool tableGet(Table* table, ObjString* key, Value* value);

/**
 * Adds the provided key-value pair to the specified hash table.
 * Returns true if the entry was successfully added and false otherwise.
*/
bool tableSet(Table* table, ObjString* key, Value value);

/**
 * Delete string object from the table.
*/
bool tableDelete(Table* table, ObjString* key);

/**
 * Copying entries of one hash table into another with respect to a new hash code
 * for determining positions for entries in the target hash table.
*/
void tableAddAll(Table* from, Table* to);

/**
 * Looks for a string in the given table and checks for equivalence.
*/
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);
#endif