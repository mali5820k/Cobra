#ifndef kc_value_h
#define kc_value_h

#include "common.h"

// Forward declaration of the Obj struct, the definition is included in the object.h
typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
}   ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj; // Anything that is stored on the heap is classified as an object.
    } as;
} Value;

/**
 * Macros that check a value's type to ensure a value is of a specific type before accessing it
 * via the AS_ macros. This ultimately allows us to use AS macros safely without errors.
*/ 
#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NULL(value)      ((value).type == VAL_NULL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)       ((value).type == VAL_OBJ)

/**
 * AS_ macros extract the value from the value field of the instruction based upon the value's
 * type. 
 * 
 * More specifics are listed below:
 * 
 * Unpack the variable types and get the values that they hold to be
 * able to use them in any way. These access the union fields directly
 * so these methods/macros should be used with caution, so only use
 * these if you know exactly what the type is for a variable.
*/
#define AS_OBJ(value)       ((value).as.obj)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)


/**
 * These macros construct the "byte code" for each respective type.
 * Specifically, these macros pack the variable types with their value, which allows us to use
 * static allocation means for what appears to be a dynamic allocation
 * to the user.
*/
#define BOOL_VAL(value)     ((Value){VAL_BOOL, {.boolean = value}})
#define NULL_VAL            ((Value){VAL_NULL, {.number = 0}})
#define NUMBER_VAL(value)   ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)     ((Value){VAL_OBJ, {.obj = (Obj*)object}})

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b); // Returns a C-bool for whether or not two values are equal.
void initValueArray(ValueArray* array); // Initializes a value array.
void writeValueArray(ValueArray* array, Value value); // Writes a value to the specified value array.
void freeValueArray(ValueArray* array); // Frees the specified value array from memory
void printValue(Value value); // Prints out the specified value.


#endif