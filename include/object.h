#pragma once

#include "common.h"
#include "value.h"

enum ObjType {
    OBJ_STRING,
};

struct Obj {
    ObjType type;
    Obj* next;
};

struct ObjString : Obj {
    int length;
    char* chars;
};

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

inline ObjType objType(Value v) {return asObj(v)->type;}

inline bool isObjType(Value v, ObjType type) {return isObj(v) && asObj(v)->type == type;}
inline bool isString(Value v) {return isObjType(v, OBJ_STRING);}

inline ObjString* asString(Value v) {return static_cast<ObjString*>(asObj(v));}
inline char* asCString(Value v) {return static_cast<ObjString*>(asObj(v))->chars;}