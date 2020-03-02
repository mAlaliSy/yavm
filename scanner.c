#include "scanner.h"

#include <stdio.h>    
#include <string.h>   

#include "commons.h"   
#include "scanner.h"  

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;


Scanner scanner;


static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}


static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}


static bool isAtEnd() {
    return *scanner.current == '\0';
}

void initScanner(const char* source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

Token scanToken() {
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    return errorToken("Unexpected character.");
}