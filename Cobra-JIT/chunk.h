#ifndef cbr_chunk_h
#define cbr_chunk_h

#include "common.h"

// OpCode is the one-byte operation code
typedef enum {
    OP_RETURN,
} OpCode;

// A dynamic array of instructions
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);

#endif
