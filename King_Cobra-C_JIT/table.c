#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

/**
 * The max load factor of the hash table before growing in size.
*/
#define TABLE_MAX_LOAD 0.75

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

/**
 * Finds and returns the pointer reference to the entry from the given key.
 * If there is no key entry at a specific index, then that is an open spot for new entries to be
 * inserted into.
 * 
 * This function is the core of the hash table that deals with linear probing and 
 * collision handling.
*/
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key -> hash % capacity;
    // Linear probing is handled with the assignment of a new index.
    // The probing will wrap around to the start of the hash table due
    // to the ```(index + 1) % capacity``` calculation.
    for(;;) {
        Entry* entry = &entries[index];
        if(entry -> key == key || entry -> key == NULL) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

/**
 * Changes the size of the table (generally increasing capacity) 
 * and hashes over all previously hashed objects to new locations
 * for better uniformity and load balancing.
*/
static void adjustCapacity(Table* table, int capacity) {
    Entry* entries = ALLOCATE(Entry, capacity);
    for(int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }

    for(int i = 0; i < table -> capacity; i++) {
        Entry* entry = &table -> entries[i];
        if(entry -> key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry -> key);
        dest -> key = entry -> key;
        dest -> value = entry -> value;
    }
    
    FREE_ARRAY(Entry, table -> entries, table -> capacity);
    table -> entries = entries;
    table -> capacity = capacity;
}


/**
 * Adds the provided key-value pair to the specified hash table.
 * Returns true if the entry was successfully added and false otherwise.
*/
bool tableSet(Table* table, ObjString* key, Value value) {
    // Allocate the Entry array and ensure that the size is suitable for adding a new entry
    if(table -> count + 1 > table -> capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table -> capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table -> entries, table -> capacity, key);
    bool isNewKey = entry -> key == NULL;
    if(isNewKey) table -> count++;

    entry -> key = key;
    entry -> value = value;
    return isNewKey;
}

/**
 * Copying entries of one hash table into another with respect to a new hash code
 * for determining positions for entries in the target hash table.
*/
void tableAddAll(Table* from, Table* to) {
    for(int i = 0; i < from -> capacity; i++) {
        Entry* entry = &from -> entries[i];
        if(entry -> key != NULL) {
            tableSet(to, entry -> key, entry -> value);
        }
    }
}