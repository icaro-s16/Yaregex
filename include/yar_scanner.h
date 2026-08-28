#pragma once 

#include <stdint.h>
#include "vector.h"
#include "yar_err_flags.h"

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
    TOKEN_STRING_SIZE           = 10,
    SCANNER_EMPTY_TAPE          = 1
};

typedef enum{
    #define X(token_name, token_symbol) token_name, 
        CHAR_TOKENS(X)
        STR_TOKENS(X)
    #undef X
    CONCAT,
    SYMBOL,
    EOT
}TokenClass;

typedef struct{
    TokenClass class;
    union{
        struct{
            int start, end;
        };
        char ch;
    };
}Token;

typedef struct{
    uint8_t state;
    char    *tape; 
    Vector  *tokens; 
    Token   *last_token;
    size_t  index;
    size_t  tape_size;
}Scanner;

#ifdef YAR_DEBUG

const char* yar_token_to_string(Token token);

#endif

Vector* yar_scan(const char *pattern, uint8_t *err);

int is_terminal_token(const Token token);

int is_quantifier_token(const Token token);

static Scanner scanner_construct(const char *pattern);

static void scanner_destroy(Scanner *scanner);

static int scanner_consume(Scanner *scanner);

static void scanner_append_symbol(Scanner *scanner, Token symbol);