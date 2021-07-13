#include <stdlib.h>
#include "chunk.h"
#include "memory.h"

/**
 * Initializes an empty Chunk with no memory allocation.
 * This is basically a take on constructing bytecode
 * Similar to how JVM languages compile down to bytecode or
 * an intermediate representation.
*/
void initChunk(Chunk* chunk) {
    chunk -> count = 0;
    chunk -> capacity = 0;
    chunk -> code = NULL;
    chunk -> lines = NULL;
    initValueArray(&chunk -> constants);
}

/**
 * Frees the memory allocated for a Chunk on the heap and 
 * leaves the Chunk as it was first initialized without any
 * memory allocated for it on the heap.
*/
void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk -> code, chunk -> capacity);
    FREE_ARRAY(int, chunk -> lines, chunk -> capacity);
    freeValueArray(&chunk -> constants);
    initChunk(chunk);
}

/**
 * Writes the provided byte and the line number as well to the specified Chunk.
*/
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if(chunk -> capacity < chunk -> count + 1) {
        int oldCapacity = chunk -> capacity;
        chunk -> capacity = GROW_CAPACITY(oldCapacity);
        chunk -> code = GROW_ARRAY(uint8_t, chunk -> code, oldCapacity, chunk -> capacity);
        chunk -> lines = GROW_ARRAY(int, chunk -> lines, oldCapacity, chunk -> capacity);
    }

    chunk -> code[chunk -> count] = byte;
    chunk -> lines[chunk -> count] = line;
    chunk -> count++;
}

/**
 * Adds the provided constant instruction to the specified Chunk.
*/
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk -> constants, value);
    return chunk -> constants.count -1;
}
