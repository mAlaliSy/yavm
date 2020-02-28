#include "vm.h"
#include "commons.h"
VM vm;

initVM()
{
}

freeVM()
{
}
static InterpretResult run() {
#define READ_BYTE() (*vm.pc++)

    for (;;) {
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE                        
}

InterpretResult interpretChunk(Chunk* chunk)
{
	vm.chunk = chunk;
	// Point to the first instruction in the chunk
	vm.pc = vm.chunk->code;
	return run();
}
