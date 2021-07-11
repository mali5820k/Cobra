#ifndef kc_value_h
#define kc_value_h

#include "common.h"

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
}   ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
} Value;

// Checks a value's type so we can use AS macros safely
#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NULL(value)      ((value).type == VAL_NULL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)

// Unpack the variable types and get the values that they hold to be
// able to use them in any way. These access the union fields directly
// so these methods/macros should be used with caution, so only use
// these if you know exactly what the type is for a variable.
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)

// Packs the variable types with their value, which allows us to use
// static allocation means for what appears to be a dynamic allocation
// to the user
#define BOOL_VAL(value)     ((Value){VAL_BOOL, {.boolean = value}})
#define NULL_VAL            ((Value){VAL_NULL, {.number = 0}})
#define NUMBER_VAL(value)   ((Value){VAL_NUMBER, {.number = value}})

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