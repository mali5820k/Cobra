#ifndef kc_debug_h
#define kc_debug_h

#include "chunk.h"

// Disassembles instructions to debug
// The instructions are constructed and contained within
// Chunk objects, thus the below methods decode them
void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);
static int simpleInstruction(const char* name, int offset);
#endif