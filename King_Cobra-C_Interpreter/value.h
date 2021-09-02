#ifndef kc_value_h
#define kc_value_h

#include <string.h>
#include "common.h"

/** 
 * Forward declarations of the Obj and ObjString structs; the definitions are included in the object.h file.
*/
typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjList ObjList;

#ifdef NAN_BOXING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN     ((uint64_t)0x7ffc000000000000)

#define TAG_NULL    1
#define TAG_FALSE   2
#define TAG_TRUE    3

typedef uint64_t Value;

#define IS_BOOL(value)      (((value) | 1) == TRUE_VAL)
#define IS_NULL(value)      ((value) == NULL_VAL)
#define IS_NUMBER(value)    (((value) & QNAN) != QNAN)
#define IS_OBJ(value)       (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value)      ((value) == TRUE_VAL)
#define AS_NUMBER(value)    valueToNum(value)
#define AS_OBJ(value)       ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(b)         ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL           ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL            ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NULL_VAL            ((Value)(uint64_t)(QNAN | TAG_NULL))
#define NUMBER_VAL(num)     numToValue(num)
#define OBJ_VAL(obj)        (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

/**
 * Value types that will be used in the language are listed here.
*/
typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
}   ValueType;

/**
 * Values contain information for a type to be stored in chunks.
*/
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

#endif

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

/**
 * Returns a C-bool for whether or not two values are equal.
*/
bool valuesEqual(Value a, Value b);

/**
 * Initializes an empty value array with no memory allocation.
*/
void initValueArray(ValueArray* array);

/**
 * Writes the specified value to the provided value array,
 * and increases the size of the array with increased memory allocation
 * when necessary.
*/
void writeValueArray(ValueArray* array, Value value);

/**
 * Safely frees the memory allocated for the provided value array.
*/
void freeValueArray(ValueArray* array);

/**
 * Prints out values with respect to each variable and object type.
*/
void printValue(Value value);

#endif