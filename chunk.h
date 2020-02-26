#ifndef chunk_h
#define chunk_h

#include "commons.h"
#include "value.h"

// Operation code 
typedef enum {
	OP_CONSTANT,
	OP_RETURN,

} Opcode;


typedef struct {
	// count of bytes allocated
	int count;
	// total capacity of the array
	int capacity;
	// bytes of code
	uint8_t* code;

	ValueArray constants;

} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chun);
void writeChunk(Chunk* chunk, uint8_t byte);
int addConstant(Chunk* chunk, Value value);


#endif // !chunk_h
