#include <ctype.h>
#include "yar_token_handler.h"

static uint str_to_uint(
    Scanner *scanner
){
    uint val = 0;
    char curr = scanner->tape[
        scanner->index
    ];

    while (
        isdigit(
            curr
        )
    ){
        val = val * 10 + (curr - 48); 
        curr = scanner->tape[
            ++scanner->index
        ];
    }

    return val;
}

static void append_number_symbols(
    const Scanner *scanner,
    Vector **symbols
){
    uint initial_idx = scanner->index;

    for(
        char curr = scanner->tape[
            initial_idx
        ];
        isdigit(curr) && curr != '\0';
    ){  
        Token symbol = {
            .class  = SYMBOL,
            .attr   = CHAR, 
            .ch     = curr
        };

        handler_append_symbol(
            symbols,
            symbol
        );

        curr = scanner->tape[
            ++initial_idx
        ];
    }

}

static void handler_append_symbol(
    Vector **tokens, 
    Token symbol
){

    assert(
        is_terminal_token(symbol)
    );

    Token concat = {
        .class  = CONCAT,
        .attr   = NONE 
    };

    if (
        vector_get_size(
            *tokens
        ) 
    ) vector_append(
        tokens,
        &concat
    );

    vector_append(
        tokens,
        &symbol
    );
}

Token char_tokens_handler(
    Scanner *scanner
){
    assert(
        scanner->tape
    );
    
    char ch = scanner->tape[
            scanner->index
        ];

    switch (
        ch
    )
    {
    #define X(token_name, token_symbol)     \
        case token_symbol:                  \
            return (Token){                 \
                .class      = token_name,   \
                .attr       = NONE,         \
            };
    
    CHAR_TOKENS(X)

    #undef X

    default:
        return (Token){
            .class      = SYMBOL,
            .attr       = CHAR,
            .ch         = ch  
        };
    }
}

#define RETURN_INVALID_OP_TOKEN(scanner, symbols, symbol)   \
    vector_concat(                                          \
            &scanner->tokens,                               \
            symbols                                         \
        );                                                  \
        if (                                                \
            vector_get_size(scanner->tokens) > 0            \
        ){                                                  \
            scanner->last_token = vector_get(               \
                scanner->tokens,                            \
                vector_get_size(scanner->tokens) - 1        \
            );                                              \
        }                                                   \
        vector_destroy(                                     \
            symbols                                         \
        );                                                  \
        symbol.ch = curr;                                   \
        if (                                                \
            curr == '\0'                                    \
        ) {                                                 \
            symbol.class = EOT;                             \
            symbol.attr  = NONE;                            \
        }                                                   \
        return symbol

Token ranged_char_handler(
    Scanner *scanner
){
    assert(
        scanner &&
        scanner->tape
    );

    Token symbol = {
        .class  = SYMBOL,
        .attr   = CHAR
    };

    Vector *symbols = vector_construct(
        sizeof(Token)
    );
    Token token = {
        .class = RANGED_CHAR,
        .attr  = RANGE
    };
    char curr = scanner->tape[
        scanner->index
    ];

    if (
        curr != '['
    ){
        symbol.ch = curr;
        if (
            curr == '\0'
        ) {
            symbol.class = EOT;
            symbol.attr  = NONE;
        }
        vector_destroy(
            symbols
        );
        return symbol;
    }

    symbol.ch = curr;
    handler_append_symbol(
        &scanner->tokens,
        symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isalpha(curr)
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }

    symbol.ch = curr;
    handler_append_symbol(
        &scanner->tokens,
        symbol
    );

    token.start = (int)curr;
    curr = scanner->tape[
        ++scanner->index
    ];
    if (
        curr != '-'
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }

    symbol.ch = curr;
    handler_append_symbol(
        &scanner->tokens,
        symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isalpha(curr)
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }

    symbol.ch = curr;
    handler_append_symbol(
        &scanner->tokens,
        symbol
    );
    token.end = (int)curr;
    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        curr != ']'
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }
    vector_destroy(
        symbols
    );
    return token;
}

Token quantifier_handler(
    Scanner *scanner
){
    assert(
        scanner && 
        scanner->tape 
    );
    Token token = {
        .attr = RANGE
    };

    Vector *symbols = vector_construct(
        sizeof(Token)
    );
    Token symbol = {
        .class  = SYMBOL,
        .attr   = CHAR
    };

    char curr = scanner->tape[
        scanner->index
    ];
    if (
        curr != '{'
    ) {
        symbol.ch = curr;
        vector_destroy(
            symbols
        );
        symbol.ch = curr;
        if (
            curr == '\0'
        ) {
            symbol.class = EOT;
            symbol.attr  = NONE;
        }
        return symbol;
    }
    
    symbol.ch = curr; 
    handler_append_symbol(
        &scanner->tokens,
        symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isdigit(curr)
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }
    append_number_symbols(
        scanner,
        &symbols
    );
    token.start = str_to_uint(
        scanner
    );
    curr = scanner->tape[
        scanner->index
    ];
    if (
        curr == '}'
    ){
        vector_destroy(
            symbols
        );
        token.class = QUANTIFIER_EXACT;
        return token;
    }
    if (
        curr != ','
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }
    
    symbol.ch = curr; 
    handler_append_symbol(
        &scanner->tokens,
        symbol
    );
    curr = scanner->tape[
        ++scanner->index
    ];
    if (
        curr == '}'
    ){  
        vector_destroy(
            symbols
        );
        token.class = QUANTIFIER_MIN;
        return token;
    }
    if (
        !isdigit(curr)
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }
    
    append_number_symbols(
        scanner,
        &symbols
    );
    token.end = str_to_uint(
        scanner
    );
    curr = scanner->tape[
        scanner->index
    ];
    if (
        curr != '}'
    ){
        RETURN_INVALID_OP_TOKEN(
            scanner,
            symbols,
            symbol
        );
    }
    vector_destroy(
        symbols
    );

    token.class = RANGED_QUANTIFIER;
    return token;
}

#undef RETURN_INVALID_OP_TOKEN

Token backslash_handler(
    Scanner *scanner
){
    assert(
        scanner && 
        scanner->tape
    );

    char curr = scanner->tape[
        scanner->index
    ];

    Token symbol = {
        .class  = SYMBOL,
        .attr   = CHAR
    };

    if (
        curr != '\\'
    ) {
        symbol.ch = curr;
        return symbol;
    };

    curr = scanner->tape[
        ++scanner->index
    ];

    Token token = {0};
    switch(
        curr
    ){
    case 'd':
        token.class = ANY_DIGIT;
        token.attr  = NONE;
        return token; 
    case 'D':
        token.class = ANY_NON_DIGIT;
        token.attr  = NONE;
        return token;
    case 's':
        token.class = ANY_WHITESPACE;
        token.attr  = NONE;
        return token;
    case 'S':
        token.class = ANY_NON_WHITESPACE;
        token.attr  = NONE;
        return token;
    case '*': case '+': case '\\': case '|': case '.': case '(': 
    case ')': case '^': case '$': case '[': case ']': case '{':
    case '}':
        token.ch    = curr;
        return token;
    default:
        symbol.ch = '\\';
        vector_append(
            &scanner->tokens,
            &symbol
        );       
        symbol.ch = curr;
        if (
            curr == '\0'
        ){
            token.attr  = NONE;
            token.class = EOT;
        }
        return symbol;
    }
}
