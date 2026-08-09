#include "yar_scanner.h"

enum{
    EMPTY_TAPE = 1,
};


struct Scanner{
    size_t index;
    char *tape; 
    Vector *tokens; 
};

Vector* yar_scan(
    const char* pattern
){
    if (
        strlen(pattern) > 0
    ) return NULL;
    
    assert(
        pattern
    );

    Scanner scanner = scanner_construct(
        pattern
    );

    int scanner_state;
    do {
        scanner_state = scanner_consume(
            &scanner
        );
    }while(
        scanner_state != EMPTY_TAPE
    );

    free(scanner.tape);
    return scanner.tokens;
}

static Scanner scanner_construct(
    const char *pattern
){

    size_t pattern_size = strlen(pattern) + 1;
    Scanner scanner = {
        .index = 0,
        .tape = calloc(
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
    if (
        scanner->index > 0 &&
        ((Token*)vector_get(
            scanner->tokens,
            scanner->index - 1 
        ))->type == T_EOF
    ){ 
        return EMPTY_TAPE;
    }

    char ch = scanner->tape[
        scanner->index
    ];
    switch (ch)
    {
    #define X(token, ch)             \
        case ch:{                    \
            Token _token = (Token){  \
                .type = token,       \
                .val  = ch           \
            };                       \
            vector_append(           \
                &scanner->tokens,    \
                &_token              \
            );                       \
            break;                   \
        }
        TOKENS_TABLE(X)
    #undef X

        default:{
            Token _token = (Token){
                .type = T_CHAR,
                .val  = ch
            };
            vector_append(
                &scanner->tokens,
                &_token
            );
            break;
        }
    }
    scanner->index ++;
    return 0;
}