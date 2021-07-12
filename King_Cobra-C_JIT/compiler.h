#ifndef kc_compiler_h
#define kc_compiler_h

#include "object.h"
#include "vm.h"

/**
 * The function that calls and pieces the functions of the compiler together and runs.
 * Think of this as the "main" method of the compiler.
*/
bool compile(const char* source, Chunk* chunk);

#endif