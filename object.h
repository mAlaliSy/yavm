//
// Created by malal on 3/15/2020.
//

#ifndef YAVM_OBJECT_H
#define YAVM_OBJECT_H
#include "commons.h"
#include "value.h"


#define OBJ_TYPE(value)         (AS_OBJ(value)->type)

#define IS_STRING(value)        isObjType(value, OBJ_STRING)

#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)


typedef enum {
    OBJ_STRING,
} ObjType;

struct sObj {
    ObjType type;
};


struct sObjString {
    Obj obj;
    int length;
    char* chars;
};

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //YAVM_OBJECT_H
