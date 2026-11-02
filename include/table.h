#pragma once

#include "common.h"
#include "value.h"

struct Entry {
    ObjString* key = nullptr;
    Value value;
};

class Table {
    public:
        Table() = default;
        ~Table() {clear();}
        Table(const Table&) = delete;
        Table& operator=(const Table&) = delete;

        bool set(ObjString* key, Value value);
        bool get(ObjString* key, Value* value) const;
        bool remove(ObjString* key);
        void addAll(const Table& from);
        ObjString* findString(const char* chars, int length, std::uint32_t hash) const;
        void clear();
    private:
        void adjustCapacity(int capacity);

        Entry* entries_ = nullptr;
        int count_ = 0;
        int capacity_ = 0;
};

// struct Table {
//     int count = 0;
//     int capacity = 0;
//     Entry* entries = nullptr;
// };

void initTable(Table* table);
void freeTable(Table* table);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableDelete(Table* table, ObjString* key);
void tableAddAll(Table* from, Table* to);
ObjString* tableFindString(Table* table, const char* chars, int length, std::uint32_t hash);