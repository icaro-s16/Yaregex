#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "yar_scanner.h"

/* NOTE: Foward declaration from "yar_token_handler.h" */

Token char_tokens_handler(Scanner *scanner);

Token char_range_handler(Scanner *scanner);

Token quantifier_handler(Scanner *scanner);

Token backslash_handler(Scanner *scanner);

#ifdef YAR_DEBUG

/* NOTE: This function is unsafe and can cause
 * overflow, so it should be used just in debug 
 * cases.
 */
const char* yar_token_to_string(
    Token token
){
    #define TO_STR(X) #X

    enum{
        BUFFER_SIZE = 50
    };
    
    #define X(token_name, token_symbol)                     \
        static char* token_name##_VAR = TO_STR(token_name);

        CHAR_TOKENS(X)
    
    #undef X 

    static char *ANY_DIGIT_VAR          = TO_STR(ANY_DIGIT);
    static char *ANY_NON_DIGIT_VAR      = TO_STR(ANY_NON_DIGIT);
    static char *ANY_WHITESPACE_VAR     = TO_STR(ANY_WHITESPACE);
    static char *ANY_NON_WHITESPACE_VAR = TO_STR(ANY_NON_WHITESPACE);
    static char *EOT_VAR                = TO_STR(EOT);

    switch(
        token.class
    ){
    case RANGED_CHAR:
        static char CHAR_RANGE_VAR[BUFFER_SIZE];
        sprintf(
            CHAR_RANGE_VAR,
            "%s(\"[%c-%c]\")",
            TO_STR(CHAR_RANGE),
            (char)token.start,
            (char)token.end
        );
        return (const char*)CHAR_RANGE_VAR;
    case QUANTIFIER_EXACT:
        static char QUANTIFIER_EXACT_VAR[BUFFER_SIZE];
        sprintf(
            QUANTIFIER_EXACT_VAR,
            "%s(\"{%d}\")",
            TO_STR(QUANTIFIER_EXACT),
            token.start
        );
        return (const char*)QUANTIFIER_EXACT_VAR;
    case RANGED_QUANTIFIER:
        static char RANGED_QUANTIFIER_VAR[BUFFER_SIZE];
        sprintf(
            RANGED_QUANTIFIER_VAR,
            "%s(\"{%d,%d}\")",
            TO_STR(RANGED_QUANTIFIER),
            token.start,
            token.end
        );
        return (const char*)RANGED_QUANTIFIER_VAR;
    case QUANTIFIER_MIN:
        static char QUANTIFIER_MIN_VAR[BUFFER_SIZE];
        sprintf(
            QUANTIFIER_MIN_VAR,
            "%s(\"{%d,}\")",
            TO_STR(QUANTIFIER_MIN),
            token.start
        );

        return (const char*)QUANTIFIER_MIN_VAR;
    }


    switch (token.class)
    {
    #define X(token_name, token_symbol) \
        case token_name:                \
            return (const char*)token_name##_VAR;

        CHAR_TOKENS(X)

    #undef  X
    
    case ANY_DIGIT:
        return (const char*)ANY_DIGIT_VAR;
    case ANY_NON_DIGIT:
        return (const char*)ANY_DIGIT_VAR;
    case ANY_WHITESPACE:
        return (const char*)ANY_WHITESPACE_VAR;
    case ANY_NON_WHITESPACE:
        return (const char*)ANY_NON_WHITESPACE_VAR; 
    case EOT:
        return (const char*)EOT_VAR;
    default:
        static char SYMBOL_VAR[BUFFER_SIZE];
        sprintf(
            SYMBOL_VAR,
            "%s('%c')",
            TO_STR(SYMBOL),
            token.ch
        );
        return (const char*)SYMBOL_VAR;
    }

    #undef TO_STR
}

#endif

Vector* yar_scan(
    const char* pattern
){
    if (
        strlen(pattern) <= 0
    ) return NULL;
    
    assert(
        pattern
    );

    Scanner scanner = scanner_construct(
        pattern
    );

    int state;
    do {
        state = scanner_consume(
            &scanner
        );
    }while(
        state != SCANNER_EMPTY_TAPE
    );

    free(scanner.tape);

    return scanner.tokens;
}

static Scanner scanner_construct(
    const char *pattern
){

    size_t pattern_size = strlen(pattern) + 1;
    Scanner scanner = {
        .index  = 0,
        .tape_size = pattern_size - 1,
        .tape   = calloc(
            pattern_size, 
            sizeof(char)
        ),
        .tokens = vector_construct(
            sizeof(Token)
        )
    };

    assert(
        scanner.tape && 
        scanner.tokens
    );

    strcpy(
        scanner.tape, 
        pattern
    );

    return scanner;
}

static int scanner_consume(
    Scanner *scanner
){
    assert(
        scanner && 
        scanner->tape
    );
    
    if (
        scanner->index > scanner->tape_size
    ){
        return SCANNER_EMPTY_TAPE;
    }

    char curr = scanner->tape[
        scanner->index
    ];

    if (
        curr == '\0'
    ){
        Token token = (Token){
            .class      = EOT, 
            .attr       = CHAR,
            .ch         = '\0'
        };
        vector_append(
            &scanner->tokens,
            &token
        );
        return SCANNER_EMPTY_TAPE;
    }

    switch(
        curr 
    ){
    case '\\':{
        Token token = backslash_handler(scanner);
        vector_append(
            &scanner->tokens,
            &token
        );
        break;
    }
    case '[':{
        Token token = char_range_handler(scanner);
        vector_append(
            &scanner->tokens,
            &token
        );
        break;
    }
    case '{':{
        Token token = quantifier_handler(scanner);
        vector_append(
            &scanner->tokens,
            &token
        );
        break;
    }
    default:{
        Token token = char_tokens_handler(scanner);
        vector_append(
            &scanner->tokens,
            &token
        );
        break;
    }
    }
    scanner->index += 1;
    return 0;
}