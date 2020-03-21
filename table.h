//
// Created by malal on 3/20/2020.
//

#ifndef YAVM_TABLE_H
#define YAVM_TABLE_H

#include "commons.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableSet(Table* table, ObjString* key, Value value);
void tableAddAll(Table* from, Table* to);

#endif //YAVM_TABLE_H
