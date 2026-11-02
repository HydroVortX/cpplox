#include <string>
#include <cstring>

#include "include/common.h"
#include "include/memory.h"
#include "include/object.h"
#include "include/table.h"
#include "include/value.h"

#define TABLE_MAX_LOAD 0.75



static Entry* findEntry(Entry* entries, int capacity, ObjString* key){
    std::uint32_t index = key->hash % capacity;
    Entry* tombstone = nullptr;

    for(;;){
        Entry* entry = &entries[index];

        if(entry->key == nullptr){
            if(isNil(entry->value)){
                //empty entry
                return tombstone != nullptr ? tombstone : entry;
            } else{
                //tombstone found
                if(tombstone == nullptr) tombstone = entry;
            }
        } else if(entry->key == key){
            //found key
            return entry;
        }

        index = (index + 1) % static_cast<std::uint32_t>(capacity);
    }
}

void Table::clear(){
    freeArray(entries_, capacity_);
    entries_ = nullptr;
    count_ = 0;
    capacity_ = 0;
}

bool Table::get(ObjString* key, Value* value) const{
    if(count_ == 0) return false;

    Entry* entry = findEntry(entries_, capacity_, key);
    if(entry->key == nullptr) return false;

    *value = entry->value;
    return true;
}

void Table::adjustCapacity(int capacity){
    Entry* entries = allocate<Entry>(static_cast<size_t>(capacity));
    for(int i = 0; i < capacity; i++){
        entries[i].key = nullptr;
        entries[i].value = nilVal();
    }

    count_ = 0;
    for(int i = 0; i< capacity_; i++){
        Entry* entry = &entries_[i];
        if(entry->key == nullptr) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        count_++;
    }

    freeArray(entries_, capacity_);
    entries_ = entries;
    capacity_ = capacity;
}

bool Table::set(ObjString* key, Value value){
    if(count_ + 1 > capacity_ * TABLE_MAX_LOAD){
        adjustCapacity( growCapacity(capacity_));
    }

    Entry* entry = findEntry(entries_, capacity_, key);
    bool isNewKey = entry->key == nullptr;
    if(isNewKey && isNil(entry->value)) count_++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool Table::remove(ObjString* key){
    if(count_ == 0) return false;

    //find entry
    Entry* entry = findEntry(entries_, capacity_, key);
    if(entry->key == nullptr) return false;

    //place tombstone
    entry->key = nullptr;
    entry->value = boolVal(true);
    return true;
}

void Table::addAll(const Table& from){
    for(int i = 0; i < from.capacity_; i++){
        const Entry& entry = from.entries_[i];
        if(entry.key != nullptr){
            set(entry.key, entry.value);
        }
    }
}

ObjString* Table::findString(const char* chars, int length, std::uint32_t hash) const{
    if(count_ == 0) return nullptr;

    std::uint32_t index = hash % capacity_;
    for(;;){
        Entry* entry = &entries_[index];
        if(entry->key == nullptr){
            //stop if non tombstone entry
            if(isNil(entry->value)) return nullptr;
        } else if(entry->key->length == length && 
                  entry->key->hash == hash &&
                  std::memcmp(entry->key->chars, chars, static_cast<size_t>(length)) == 0){
            //fount it
            return entry->key;
        }

        index = (index + 1) % static_cast<std::uint32_t>(capacity_);
    }
}