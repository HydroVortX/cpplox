#include <string>

#include "include/common.h"
#include "include/memory.h"
#include "include/object.h"
#include "include/table.h"
#include "include/value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table){
    table->count = 0;
    table->capacity = 0;
    table->entries = nullptr;
}

void freeTable(Table* table){
    freeArray(table->entries, table->capacity);
    initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key){
    std::uint32_t index = key->hash % capacity;
    for(;;){
        Entry* entry = &entries[index];
        if(entry->key == key || entry->key == nullptr){
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table* table, int capacity){
    Entry* entries = allocate<Entry>(capacity);
    for(int i = 0; i < capacity; i++){
        entries[i].key = nullptr;
        entries[i].value = nilVal();
    }

    for(int i = 0; i< table->capacity; i++){
        Entry* entry = &table->entries[i];
        if(entry->key == nullptr) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
    }

    freeArray(table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table* table, ObjString* key, Value value){
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD){
        int capacity = growCapacity(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == nullptr;
    if(isNewKey) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

void tableAddAll(Table* from, Table* to){
    for(int i = 0; i < from->capacity; i++){
        Entry* entry = &from->entries[i];
        if(entry->key != nullptr){
            tableSet(to, entry->key, entry->value);
        }
    }
}