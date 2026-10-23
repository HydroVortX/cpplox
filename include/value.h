#pragma once

#include "common.h"

struct Obj;
struct ObjString;

enum ValueType{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
};

struct Value{
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
};

inline Value boolVal(bool b) {Value v; v.type = VAL_BOOL; v.as.boolean = b; return v;}
inline Value nilVal() {Value v; v.type = VAL_NIL; v.as.number = 0; return v;}
inline Value numberVal(double n) {Value v; v.type = VAL_NUMBER; v.as.number = n; return v;}
inline Value objVal(Obj* object) { Value v; v.type = VAL_OBJ; v.as.obj = object; return v;}

inline bool asBool(Value v) {return v.as.boolean;}
inline double asNumber(Value v) {return v.as.number;}
inline Obj* asObj(Value v) {return v.as.obj;}
 
inline bool isBool(Value v) {return v.type == VAL_BOOL;}
inline bool isNil(Value v) {return v.type == VAL_NIL;}
inline bool isNumber(Value v) {return v.type == VAL_NUMBER;}
inline bool isObj(Value v) {return v.type == VAL_OBJ;}

struct ValueArray{
    int capacity = 0;
    int count = 0;
    Value* values = nullptr;
};

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);