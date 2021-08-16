#ifndef kc_chunk_h
#define kc_chunk_h

#include "common.h"
#include "value.h"

/**
 * All opcodes account for one byte in the two bytes of an instruction.
*/
typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,   // Will implement these three instructions later
    OP_LESS_EQUAL,    // once the required instructions are tested
    OP_NOT_EQUAL,       // fully functional
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_RETURN,
} OpCode;

/**
 * Chunks store instructions within a dynamic array.
 * Each Chunk is responsible for associated instructions, lines from source code that the instructions
 * are read from, and an array of values.
*/
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

/**
 * Initializes an empty Chunk with no memory allocation.
 * This is basically a take on constructing bytecode
 * Similar to how JVM languages compile down to bytecode or
 * an intermediate representation.
*/
void initChunk(Chunk* chunk);

/**
 * Frees the memory allocated for a Chunk on the heap and 
 * leaves the Chunk as it was first initialized without any
 * memory allocated for it on the heap.
*/
void freeChunk(Chunk* chunk);

/**
 * Writes the provided byte and the line number as well to the specified Chunk.
*/
void writeChunk(Chunk* chunk, uint8_t byte, int line);

/**
 * Adds the provided constant instruction to the specified Chunk.
*/
int addConstant(Chunk* chunk, Value value);

#endif
