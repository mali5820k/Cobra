#ifndef kc_object_h
#define kc_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

// A macro that extracts the type from a specific value.
#define OBJ_TYPE(value)     (AS_OBJ(value) -> type)

/**
 * Macros to check if the provided value is of a specific type of object.
*/
#define IS_FUNCTION(value)  isObjType(value, OBJ_FUNCTION)
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

/**
 * The AS_STRING macro returns the pointer to that string object,
 * and the AS_CSTRING macro returns the character array of that string object.
*/
#define AS_FUNCTION(value)  ((ObjFunction*)AS_OBJ(value))
#define AS_STRING(value)    ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString*)AS_OBJ(value)) -> chars)

typedef enum {
    OBJ_FUNCTION,
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
};

typedef struct {
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

ObjFunction* newFunction();
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

/**
 * Using a function in the macro for IS_STRING(value) since the macro's expression gets evaluated each time a parameter
 * is used. If the return line from this function was directly included in the IS_STRING macro, that would be observed
 * behavior and that will cause unintentional behavior.
*/
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value) -> type == type;
}

#endif