#include <ctype.h>
#include "yar_token_handler.h"

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
                .attr       = CHAR,         \
                .ch         = ch            \
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

Token char_range_handler(
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
        scanner->index++
    ];

    if (
        curr != '['
    ){
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        vector_destroy(
            symbols
        );
        return symbol;
    }

    symbol.ch = curr;
    vector_append(
        &symbols, 
        &symbol
    );

    curr = scanner->tape[
        scanner->index
    ];

    if (
        !isalpha(curr)
    ){
        vector_concat(
            &scanner->tokens,
            symbols
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }

    symbol.ch = curr;
    vector_append(
        &symbols, 
        &symbol
    );

    token.start = (int)curr;
    curr = scanner->tape[
        ++scanner->index
    ];
    if (
        curr != '-'
    ){
        vector_concat(
            &scanner->tokens,
            symbols
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }

    symbol.ch = curr;
    vector_append(
        &symbols, 
        &symbol
    );
    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isalpha(curr)
    ){
        vector_concat(
            &scanner->tokens,
            symbols
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }

    symbol.ch = curr;
    vector_append(
        &symbols, 
        &curr
    );
    token.end = (int)curr;
    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        curr != ']'
    ){
        vector_concat(
            &scanner->tokens,
            symbols
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }
    vector_destroy(
        symbols
    );
    return token;
}

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

        vector_append(
            symbols,
            &symbol
        );

        curr = scanner->tape[
            ++initial_idx
        ];
    }

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
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }
    
    symbol.ch = curr; 
    vector_append(
        &symbols, 
        &symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isdigit(curr)
    ){
        vector_concat(
            &scanner->tokens, 
            symbols 
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
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
        vector_concat(
            &scanner->tokens, 
            symbols 
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }
    
    symbol.ch = curr; 
    vector_append(
        &symbols, 
        &symbol
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
        vector_concat(
            &scanner->tokens, 
            symbols 
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
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
        vector_concat(
            &scanner->tokens, 
            symbols 
        );
        vector_destroy(
            symbols
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }
    vector_destroy(
        symbols
    );

    token.class = RANGED_QUANTIFIER;
    return token;
}

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
        token.attr  = STRING;
        strcpy(token.st, "\\d");
        return token; 
    case 'D':
        token.class = ANY_NON_DIGIT;
        token.attr  = STRING;
        strcpy(token.st, "\\D");
        return token;
    case 's':
        token.class = ANY_WHITESPACE;
        token.attr  = STRING;
        strcpy(token.st, "\\s");
        return token;
    case 'S':
        token.class = ANY_NON_WHITESPACE;
        token.attr  = STRING;
        strcpy(token.st, "\\S");
        return token;
    case '*': case '+': case '\\': case '|': case '.': case '(': 
    case ')': case '^': case '$': case '[': case ']': case '{':
    case '}':
        token.class = SYMBOL;
        token.attr  = CHAR;
        token.ch    = curr;
        return token;
    default:
        symbol.ch = '\\';
        vector_append(
            &scanner->tokens,
            &symbol
        );
        symbol.class = (
            curr == '\0'
        ) ? EOT : SYMBOL;
        symbol.ch = curr;
        return symbol;
    }
}
