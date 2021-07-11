#ifndef kc_chunk_h
#define kc_chunk_h

#include "common.h"
#include "value.h"

// OpCode is the one-byte operation code
typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,   // Will implment these three instructions later
    OP_LESS_EQUAL,    // once the required instructions are tested and
    OP_NOT_EQUAL,       // fully functional
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN,
} OpCode;

// A dynamic array of instructions
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif
