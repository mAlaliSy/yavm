#include "chunk.h"
#include "value.h"
#include "table.h"

#ifndef VM_H
#define VM_H
#define MAX_STACK 256
typedef struct {
	Chunk* chunk;
    // Program counter
    uint8_t* pc;

    Value stack[MAX_STACK];
    Value* stackTop;

    Obj* objects;

    Table strings;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


void initVM();
void freeVM();

InterpretResult interpret(const char* code);

extern VM vm;

void push(Value value);
Value pop();

#endif // !VM_H
