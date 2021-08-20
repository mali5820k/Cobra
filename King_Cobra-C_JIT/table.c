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
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

/**
 * Frees the memory and all allocated entries in the specified Hash table.
*/
void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
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
    uint32_t index = key->hash & (capacity - 1);
    Entry* tombstone = NULL;
    // Linear probing is handled with the assignment of a new index.
    // The probing will wrap around to the start of the hash table due
    // to the ```(index + 1) % capacity``` calculation.
    for(;;) {
        Entry* entry = &entries[index];
        if(entry->key == NULL) {
            if(IS_NULL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            }
            else {
                if(tombstone == NULL) tombstone = entry;
            }
        }
        else if(entry->key == key) {
            return entry;
        }
        
        index = (index + 1) & (capacity - 1);
    }
}

/**
 * Get the value associated with the provided key and sets the value variable
 * that's passed in to point to that value. Finally, the function returns true
 * if the value is found, otherwise false.
*/
bool tableGet(Table* table, ObjString* key, Value* value) {
    if(table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if(entry->key == NULL) return false;

    *value = entry->value;
    return true;
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

    table->count = 0;
    for(int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if(entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}


/**
 * Adds the provided key-value pair to the specified hash table.
 * Returns true if the entry was successfully added and false otherwise.
*/
bool tableSet(Table* table, ObjString* key, Value value) {
    // Allocate the Entry array and ensure that the size is suitable for adding a new entry
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if(isNewKey && IS_NULL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

/**
 * Delete string object from the table.
*/
bool tableDelete(Table* table, ObjString* key) {
    // If the table is empty, bail
    if(table->count == 0) return false;

    // Find the entry associated with the specified key
    Entry* entry = findEntry(table->entries, table->capacity, key);
    // If there is no entry there or a tombstone, bail
    if(entry->key == NULL) return false;

    // Otherwise, we will set at that entries' position a tombstone.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

/**
 * Copying entries of one hash table into another with respect to a new hash code
 * for determining positions for entries in the target hash table.
*/
void tableAddAll(Table* from, Table* to) {
    for(int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if(entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash) {
    if(table->count == 0) return NULL;

    uint32_t index = hash & (table->capacity - 1);

    for(;;) {
        Entry* entry = &table->entries[index];
        if(entry->key == NULL) {
            if(IS_NULL(entry->value)) return NULL;
        }
        else if(entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }
        index = (index + 1) & (table->capacity - 1);
    }
}

void tableRemoveWhite(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked) {
            tableDelete(table, entry->key);
        }
    }
}

void markTable(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        markObject((Obj*)entry->key);
        markValue(entry->value);
    }
}