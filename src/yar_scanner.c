#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "yar_scanner.h"

/* NOTE: Private foward declaration from "yar_token_handler.h" */

Token char_tokens_handler(Scanner *scanner);

Token ranged_char_handler(Scanner *scanner);

Token quantifier_handler(Scanner *scanner);

Token backslash_handler(Scanner *scanner);

#ifdef YAR_DEBUG

/* 
 * NOTE: This function is unsafe and can cause
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
    static char *CONCAT_VAR             = TO_STR(CONCAT);

    static char RANGED_CHAR_VAR         [BUFFER_SIZE];
    static char QUANTIFIER_EXACT_VAR    [BUFFER_SIZE];
    static char RANGED_QUANTIFIER_VAR   [BUFFER_SIZE];
    static char QUANTIFIER_MIN_VAR      [BUFFER_SIZE];
    static char SYMBOL_VAR              [BUFFER_SIZE];

    switch(
        token.class
    ){
    case RANGED_CHAR:
        sprintf(
            RANGED_CHAR_VAR,
            "%s(\"[%c-%c]\")",
            TO_STR(RANGED_CHAR),
            (char)token.start,
            (char)token.end
        );
        return (const char*)RANGED_CHAR_VAR;
    case QUANTIFIER_EXACT:
        sprintf(
            QUANTIFIER_EXACT_VAR,
            "%s(\"{%d}\")",
            TO_STR(QUANTIFIER_EXACT),
            token.start
        );
        return (const char*)QUANTIFIER_EXACT_VAR;
    case RANGED_QUANTIFIER:
        sprintf(
            RANGED_QUANTIFIER_VAR,
            "%s(\"{%d,%d}\")",
            TO_STR(RANGED_QUANTIFIER),
            token.start,
            token.end
        );
        return (const char*)RANGED_QUANTIFIER_VAR;
    case QUANTIFIER_MIN:
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
    case CONCAT:
        return (const char*)CONCAT_VAR;
    default:
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
    const char* pattern,
    uint8_t     *err
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

    if (
        scanner.state
    ) {
        *err = scanner.state;
        return NULL;
    }

    uint8_t tape_state;
    do {
        tape_state = scanner_consume(
            &scanner
        );
        scanner.last_token = vector_get(
            scanner.tokens,
            vector_get_size(scanner.tokens) - 1
        );
    }while(
        tape_state != SCANNER_EMPTY_TAPE
    );

    scanner_destroy(
        &scanner
    );

    if (
        scanner.state 
    ) {
        assert(
            !vector_destroy(
                scanner.tokens
            )
        );

        *err = scanner.state;

        return NULL;
    }

    return scanner.tokens;
}

int is_terminal_token(
    const Token token
){
    return (
        token.class == SYMBOL              ||
        token.class == ANY_DIGIT           ||
        token.class == ANY_NON_DIGIT       ||
        token.class == ANY_WHITESPACE      ||
        token.class == ANY_NON_WHITESPACE  ||
        token.class == RANGED_CHAR         
    ) ? 1 : 0;
}

int is_quantifier_token(
    const Token token
){
    return (
        token.class == STAR              ||
        token.class == PLUS              ||
        token.class == QMARK             ||
        token.class == QUANTIFIER_EXACT  ||
        token.class == QUANTIFIER_MIN    ||
        token.class == RANGED_QUANTIFIER      
    ) ? 1 : 0;
}

static Scanner scanner_construct(
    const char *pattern
){

    size_t pattern_size = strlen(pattern) + 1;
    Scanner scanner = {
        .state      = 0,
        .index      = 0,
        .tape_size  = pattern_size - 1,
        .tape       = calloc(
            pattern_size, 
            sizeof(char)
        ),
        .last_token = NULL,
        .tokens     = vector_construct(
            sizeof(Token)
        )
    };

    if (
        !scanner.tokens || 
        !scanner.tape
    ) {
        scanner.state |= YAR_INVALID_ALLOC;
        return scanner;
    }

    strcpy(
        scanner.tape, 
        pattern
    );

    return scanner;
}

static void scanner_destroy(
    Scanner *scanner
){
    assert(
        scanner &&
        scanner->tape
    );

    free(
        scanner->tape
    );
    scanner->tape = NULL;

    scanner->last_token = NULL;
}

static int scanner_consume(
    Scanner *scanner
){
    assert(
        scanner && 
        scanner->tape
    );
    
    if (
        scanner->index > scanner->tape_size ||
        scanner->state
    ) return SCANNER_EMPTY_TAPE;

    char curr = scanner->tape[
        scanner->index
    ];

    int err = 0;

    if (
        curr == '\0'
    ){
        Token token = (Token){
            .class      = EOT, 
            .ch         = '\0'
        };
        err = vector_append(
            &scanner->tokens,
            &token,
            sizeof(Token)
        );

        if (
            err
        ) goto invalid_vec_alloc;

        return SCANNER_EMPTY_TAPE;
    }

    switch(
        curr 
    ){
    case '\\':{
        Token token = backslash_handler(
            scanner
        );
        if (
            is_terminal_token(token)
        )  {
            scanner_append_symbol(
                scanner,
                token
            );
        }
        else{
            err = vector_append(
                &scanner->tokens,
                &token,
                sizeof(Token)
            );

            if (
                err 
            ) goto invalid_vec_alloc;
                
        }

        break;
    }
    case '[':{
        Token token = ranged_char_handler(
            scanner
        );
        if (
            is_terminal_token(token)
        )  {
            scanner_append_symbol(
                scanner,
                token
            );
        }
        else{
            err = vector_append(
                &scanner->tokens,
                &token,
                sizeof(Token)
            );

            if (
                err 
            ) goto invalid_vec_alloc;
        }
        break;
    }
    case '{':{
        Token token = quantifier_handler(
            scanner
        );
        if (
            is_terminal_token(token)
        )  {
            scanner_append_symbol(
                scanner,
                token
            );
        }
        else{
            err = vector_append(
                &scanner->tokens,
                &token,
                sizeof(Token)
            );

            if (
                err 
            ) goto invalid_vec_alloc;
        }
        
        break;
    }
    default:{
        Token concat = {
            .class = CONCAT
        };

        Token token = char_tokens_handler(
            scanner
        );
        scanner_append_symbol(
            scanner,
            token
        );
        break;
    }
    }
    scanner->index += 1;
    return 0;

    invalid_vec_alloc:
        
        assert(
            err == VEC_RESIZE_FAILED 
        );

        scanner->state |= VEC_RESIZE_FAILED;
        
        return SCANNER_EMPTY_TAPE;
}

static void scanner_append_symbol(
    Scanner *scanner, 
    Token symbol
){
    
    Token concat = {
        .class  = CONCAT
    };

    int err = 0;

    /* 
     * Concatenation cases:
     * 
     * 1. symbol concat symbol
     * 2. quantifier concat symbol
     * 3. grouping concat symbol
     * 4. grouping concat grouping
     * 5. symbol concat grouping
     */
    if (
        scanner->last_token && 
        (
            (
                (
                    scanner->last_token->class == CLOSE_PARENTHESES ||
                    is_terminal_token(*scanner->last_token)         ||
                    is_quantifier_token(*scanner->last_token)
                )&&
                (
                    is_terminal_token(symbol)                       ||
                    symbol.class == OPEN_PARENTHESES
                )
            ) 
        )
    ){
        err = vector_append(
            &scanner->tokens,
            &concat,
            sizeof(Token)
        );
    }

    if (
        err 
    ) goto invalid_vec_alloc;

    err = vector_append(
        &scanner->tokens,
        &symbol,
        sizeof(Token)
    );
    if (
        err 
    ) goto invalid_vec_alloc;

    scanner->last_token = vector_get(
        scanner->tokens,
        vector_get_size(scanner->tokens) - 1
    );

    return;

    invalid_vec_alloc:
        
        assert(
            err == VEC_RESIZE_FAILED 
        );
        
        scanner->state |= VEC_RESIZE_FAILED;
        
        return;

}