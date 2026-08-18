#pragma once 

#include "vector.h"

#define CHAR_TOKENS(X)          \
    X(OPEN_PARENTHESES, '(')    \
    X(CLOSE_PARENTHESES, ')')   \
    X(CIRCUMFLEX, '^')          \
    X(DOLAR, '$')               \
    X(PLUS, '+')                \
    X(QMARK, '?')               \
    X(STAR, '*')                \
    X(PIPE, '|')                \
    X(WILDCARD, '.')            

#define STR_TOKENS(X)               \
    X(RANGED_CHAR, "[%c-%c]")       \
    X(ANY_DIGIT, "\\d")             \
    X(ANY_NON_DIGIT, "\\D")         \
    X(ANY_WHITESPACE, "\\s")        \
    X(ANY_NON_WHITESPACE, "\\S")    \
    X(QUANTIFIER_EXACT, "{%d}")     \
    X(RANGED_QUANTIFIER, "{%d,%d}") \
    X(QUANTIFIER_MIN, "{%d,}")      

enum{
    TOKEN_STRING_SIZE     = 10,
    SCANNER_EMPTY_TAPE    = 1,
    SCANNER_INVALID_TOKEN = 2
};

typedef enum{
    #define X(token_name, token_symbol) token_name, 
        CHAR_TOKENS(X)
        STR_TOKENS(X)
    #undef X 
    SYMBOL,
    EOT
}TokenClass;

typedef enum{
    CHAR,
    RANGE,
    STRING
}TokenAttr;

typedef struct{
    TokenClass class;
    TokenAttr  attr;
    union{
        char st[TOKEN_STRING_SIZE];
        struct{
            int start, end;
        };
        char ch;
    };
}Token;

typedef struct{
    int     state;
    size_t  index;
    char    *tape; 
    Vector  *tokens; 
}Scanner;

#ifdef YAR_DEBUG

const char* yar_token_to_string(Token token);

#endif

Vector* yar_scan(const char *pattern);

static Scanner scanner_construct(const char *pattern);

static void scanner_consume(Scanner *scanner);




