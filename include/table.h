#pragma once

#include "common.h"
#include "value.h"

struct Entry {
    ObjString* key = nullptr;
    Value value;
};

struct Table {
    int count;
    int capacity;
    Entry* entries;
};

void initTable(Table* table);
void freeTable(Table* table);
bool tableSet(Table* table, ObjString* key, Value value);
void tableAddAll(Table* table, Table* to);