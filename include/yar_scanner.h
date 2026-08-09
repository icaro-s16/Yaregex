#pragma once 

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vector.h"

#define TOKENS_TABLE(X)       \
    X(T_OPARENTHESES, '(')    \
    X(T_CPARENTHESES, ')')    \
    X(T_CIRCUMFLEX, '^')      \
    X(T_BACKSLASH, '\\')      \
    X(T_OBRACKET, '{')        \
    X(T_CBRACKET, '}')        \
    X(T_MINUSS, '-')          \
    X(T_DOLARS, '$')          \
    X(T_PLUSS, '+')           \
    X(T_QMARK, '?')           \
    X(T_STAR, '*')            \
    X(T_PIPE, '|')            \
    X(T_DOT, '.')             \
    X(T_EOF, '\0')

typedef enum{
    #define X(token, ch) token = ch, 
        TOKENS_TABLE(X)
    #undef X 
    T_CHAR
}TokenType;

typedef struct Token Token;
struct Token{
    TokenType type;        
    char val;
};


typedef struct Scanner Scanner;

Vector* yar_scan(const char* pattern);

static Scanner scanner_construct(const char *pattern);

static int scanner_consume(Scanner *scanner);






