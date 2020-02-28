#include "chunk.h"
#ifndef VM_H
#define VM_H

typedef struct {
	Chunk* chunk;
    // Program counter
    uint8_t* pc;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


initVM();
freeVM();
InterpretResult interpretChunk(Chunk* chunk);

#endif // !VM_H
