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


#define RETURN_INVALID_TOKEN(x)         \
    x->state = SCANNER_INVALID_TOKEN;   \
    return (Token){0}

Token char_range_handler(
    Scanner *scanner
){
    assert(
        scanner &&
        scanner->tape
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
        RETURN_INVALID_TOKEN(scanner);
    }

    curr = scanner->tape[
        scanner->index++
    ];

    if (
        !isalpha(curr)
    ){
        RETURN_INVALID_TOKEN(scanner);
    }
    token.start = (int)curr;
    curr = scanner->tape[
        scanner->index++
    ];
    if (
        curr != '-'
    ){
        RETURN_INVALID_TOKEN(scanner);
    }
    curr = scanner->tape[
        scanner->index++
    ];

    if (
        !isalpha(curr)
    ){
        RETURN_INVALID_TOKEN(scanner);
    }
    token.end = (int)curr;
    curr = scanner->tape[
        scanner->index
    ];

    if (
        curr != ']'
    ){
        RETURN_INVALID_TOKEN(scanner);
    }

    return token;
}

static uint str_to_uint(
    Scanner *scanner
){
    assert(
        scanner &&
        scanner->tape
    );

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
    char curr = scanner->tape[
        scanner->index++
    ];
    if (
        curr != '{'
    ) {
        RETURN_INVALID_TOKEN(scanner);
    }
    curr = scanner->tape[
        scanner->index
    ];
    if (
        !isdigit(curr)
    ){
        RETURN_INVALID_TOKEN(scanner);
    }
    token.start = str_to_uint(
        scanner
    );
    curr = scanner->tape[
        scanner->index++
    ];
    if (
        curr == '}'
    ){
        token.class = QUANTIFIER_EXACT;
        return token;
    }
    if (
        curr != ','
    ){
        RETURN_INVALID_TOKEN(scanner);
    }
    curr = scanner->tape[
        scanner->index
    ];
    if (
        curr == '}'
    ){
        token.class = QUANTIFIER_MIN;
        return token;
    }
    if (
        !isdigit(curr)
    ){
        RETURN_INVALID_TOKEN(scanner);
    }
    token.end = str_to_uint(
        scanner
    );
    curr = scanner->tape[
        scanner->index
    ];
    if (
        curr != '}'
    ){
        RETURN_INVALID_TOKEN(scanner);
    }

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
        scanner->index++
    ];

    if (
        curr != '\\'
    ) {
        RETURN_INVALID_TOKEN(scanner);
    };

    curr = scanner->tape[
        scanner->index
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
        RETURN_INVALID_TOKEN(scanner);
    }
}

#undef RETURN_INVALID_TOKEN