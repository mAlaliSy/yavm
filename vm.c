#include "vm.h"
#include "commons.h"
#include <stdio.h>
#include "debug.h"
#include "compiler.h"


VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM()
{
    resetStack();   
}

void freeVM()
{
}
InterpretResult interpret(const char* source)
{
    compile(source);
    return INTERPRET_OK;
}
static InterpretResult run() {
#define READ_BYTE() (*vm.pc++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)      

    while(1) {
        #ifdef DEBUG_TRACE_EXECUTION                                        
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int)(vm.pc - vm.chunk->code));
        #endif 
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NEGATE: 
                push(-pop());
                break;
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
#undef BINARY_OP    
#undef READ_CONSTANT
#undef READ_BYTE                        
}


void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}
