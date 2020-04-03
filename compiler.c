#include "compiler.h"
#include <stdio.h>
#include "commons.h"
#include "scanner.h"
#include "chunk.h"
#include <stdlib.h>
#include "object.h"

#ifdef DEBUG_PRINT_CODE

#include "debug.h"

#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;

} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =        
    PREC_OR,          // or       
    PREC_AND,         // and      
    PREC_EQUALITY,    // == !=    
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -      
    PREC_FACTOR,      // * /      
    PREC_UNARY,       // ! -      
    PREC_CALL,        // . ()     
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();


typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;


Chunk *compilingChunk;

static Chunk *currentChunk() {
    return compilingChunk;
}


static void expression();

static ParseRule *getRule(TokenType type);

static void parsePrecedence(Precedence precedence);


static void emitReturn();
static void statement();
static void declaration();

static void synchronize();

static uint8_t identifierConstant(Token* name);


static void errorAt(Token *token, const char *message) {
    if (parser.panicMode) return;

    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.                                                
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char *message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    while (1) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}
static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void endCompiler() {
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
    emitReturn();
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void binary() {
    // Remember the operator.                                
    TokenType operatorType = parser.previous.type;

    // Compile the right operand.                            
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence) (rule->precedence + 1));

    // Emit the operator instruction.                        
    switch (operatorType) {
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        default:
            return; // Unreachable.
    }
}

static void literal() {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
        default:
            return; // Unreachable.
    }
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void string() {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
}
static void namedVariable(Token name) {
    uint8_t arg = identifierConstant(&name);
    emitBytes(OP_GET_GLOBAL, arg);
}
static void variable() {
    namedVariable(parser.previous);
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand.               
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.              
    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return; // Unreachable.
    }
}

ParseRule rules[] = {
        {grouping, NULL, PREC_NONE},       // TOKEN_LEFT_PAREN
        {NULL,     NULL, PREC_NONE},       // TOKEN_RIGHT_PAREN
        {NULL,     NULL, PREC_NONE},       // TOKEN_LEFT_BRACE
        {NULL,     NULL, PREC_NONE},       // TOKEN_RIGHT_BRACE
        {NULL,     NULL, PREC_NONE},       // TOKEN_COMMA
        {NULL,     NULL, PREC_NONE},       // TOKEN_DOT
        {unary, binary,  PREC_TERM},       // TOKEN_MINUS
        {NULL,  binary,  PREC_TERM},       // TOKEN_PLUS
        {NULL,     NULL, PREC_NONE},       // TOKEN_SEMICOLON
        {NULL,  binary,  PREC_FACTOR},     // TOKEN_SLASH
        {NULL,  binary,  PREC_FACTOR},     // TOKEN_STAR
        {unary,    NULL, PREC_NONE},       // TOKEN_BANG
        {NULL,  binary,  PREC_EQUALITY},   // TOKEN_BANG_EQUAL
        {NULL,     NULL, PREC_NONE},       // TOKEN_EQUAL
        {NULL,  binary,  PREC_EQUALITY},   // TOKEN_EQUAL_EQUAL
        {NULL,  binary,  PREC_COMPARISON}, // TOKEN_GREATER
        {NULL,  binary,  PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
        {NULL,  binary,  PREC_COMPARISON}, // TOKEN_LESS
        {NULL,  binary,  PREC_COMPARISON}, // TOKEN_LESS_EQUAL
        { variable, NULL,    PREC_NONE },       // TOKEN_IDENTIFIER
        { string,   NULL,    PREC_NONE },       // TOKEN_STRING
        {number,   NULL, PREC_NONE},       // TOKEN_NUMBER
        {NULL,     NULL, PREC_NONE},       // TOKEN_AND
        {NULL,     NULL, PREC_NONE},       // TOKEN_CLASS
        {NULL,     NULL, PREC_NONE},       // TOKEN_ELSE
        {literal,  NULL, PREC_NONE},       // TOKEN_FALSE
        {NULL,     NULL, PREC_NONE},       // TOKEN_FOR
        {NULL,     NULL, PREC_NONE},       // TOKEN_FUN
        {NULL,     NULL, PREC_NONE},       // TOKEN_IF
        {literal,  NULL, PREC_NONE},       // TOKEN_NIL
        {NULL,     NULL, PREC_NONE},       // TOKEN_OR
        {NULL,     NULL, PREC_NONE},       // TOKEN_PRINT
        {NULL,     NULL, PREC_NONE},       // TOKEN_RETURN
        {NULL,     NULL, PREC_NONE},       // TOKEN_SUPER
        {NULL,     NULL, PREC_NONE},       // TOKEN_THIS
        {literal,  NULL, PREC_NONE},       // TOKEN_TRUE
        {NULL,     NULL, PREC_NONE},       // TOKEN_VAR
        {NULL,     NULL, PREC_NONE},       // TOKEN_WHILE
        {NULL,     NULL, PREC_NONE},       // TOKEN_ERROR
        {NULL,     NULL, PREC_NONE},       // TOKEN_EOF
};

static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}
static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    }else{
        expressionStatement();
    }
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;

        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                // Do nothing.
                ;
        }

        advance();
    }
}
static void defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}
static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    if (parser.panicMode) synchronize();
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

uint8_t identifierConstant(Token *name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    while (!match(TOKEN_EOF)) {
        declaration();
    }
    endCompiler();
    return !parser.hadError;
}