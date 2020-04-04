#ifndef chunk_h
#define chunk_h

#include "commons.h"
#include "value.h"

// Operation code 
typedef enum {
	OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
	OP_NEGATE,
    OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_PRINT,
    OP_POP,
    OP_RETURN,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,


} Opcode;


typedef struct {
	// count of bytes allocated
	int count;
	// total capacity of the array
	int capacity;
	// bytes of code
	uint8_t* code;
	// line number for each byte code instruction
	int* lines;

	ValueArray constants;

} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chun);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);


#endif // !chunk_h
