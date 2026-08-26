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
            };
    
    CHAR_TOKENS(X)

    #undef X

    default:
        return (Token){
            .class      = SYMBOL,
            .ch         = ch  
        };
    }
}
    
Token ranged_char_handler(
    Scanner *scanner
){
    assert(
        scanner &&
        scanner->tape
    );

    Token symbol = {
        .class  = SYMBOL
    };

    Vector *symbols = vector_construct(
        sizeof(Token)
    );
    Token token = {
        .class = RANGED_CHAR
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
        ) symbol.class = EOT;
        
        vector_destroy(
            symbols
        );
        return symbol;
    }

    symbol.ch = curr;
    handler_append_symbol(
        &symbols,
        symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isalpha(curr)
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
        );
    }

    symbol.ch = curr;
    handler_append_symbol(
        &symbols,
        symbol
    );

    token.start = (int)curr;
    curr = scanner->tape[
        ++scanner->index
    ];
    if (
        curr != '-'
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
        );
    }

    symbol.ch = curr;
    handler_append_symbol(
        &symbols,
        symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isalpha(curr)
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
        );
    }

    symbol.ch = curr;
    handler_append_symbol(
        &symbols,
        symbol
    );
    token.end = (int)curr;
    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        curr != ']'
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
        );
    }
    vector_destroy(
        symbols
    );

    scanner->state |= (
        token.start >= token.end 
    ) ?
    SCANNER_INVALID_RANGED_CHAR :
    scanner->state;

    return token;
}

Token quantifier_handler(
    Scanner *scanner
){
    assert(
        scanner && 
        scanner->tape 
    );
    Token token;

    Vector *symbols = vector_construct(
        sizeof(Token)
    );
    Token symbol = {
        .class  = SYMBOL
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
        ) symbol.class = EOT;
        
        return symbol;
    }
    
    symbol.ch = curr; 
    handler_append_symbol(
        &symbols,
        symbol
    );

    curr = scanner->tape[
        ++scanner->index
    ];

    if (
        !isdigit(curr)
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
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

        scanner->state |= (
            !token.start 
        ) ? 
        SCANNER_INVALID_QUANTIFIER : 
        scanner->state;

        token.class = QUANTIFIER_EXACT;
        return token;
    }
    if (
        curr != ','
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
        );
    }
    
    symbol.ch = curr; 
    handler_append_symbol(
        &symbols,
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

        scanner->state |= (
            !token.start 
        ) ? 
        SCANNER_INVALID_QUANTIFIER : 
        scanner->state;

        token.class = QUANTIFIER_MIN;
        return token;
    }
    if (
        !isdigit(curr)
    ){
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
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
        return handle_invalid_operation_syntax(
            scanner,
            symbols,
            curr
        );
    }
    vector_destroy(
        symbols
    );

    scanner->state |= (
        !token.start 
    ) ? 
    SCANNER_INVALID_QUANTIFIER : 
    scanner->state;

    scanner->state |= (
        token.start >= token.end
    ) ? 
    SCANNER_INVALID_RANGED_QUANTIFIER :
    scanner->state;

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
        .class  = SYMBOL
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
        return token; 
    case 'D':
        token.class = ANY_NON_DIGIT;
        return token;
    case 's':
        token.class = ANY_WHITESPACE;
        return token;
    case 'S':
        token.class = ANY_NON_WHITESPACE;
        return token;
    case '*': case '+': case '\\': case '|': case '.': case '(': 
    case ')': case '^': case '$': case '[': case ']': case '{':
    case '}':
        token.ch    = curr;
        return token;
    default:{
        Token concat = {
            .class  = CONCAT
        };
        if (
            scanner->last_token &&
            is_terminal_token(*scanner->last_token)
        ){
            vector_append(
                &scanner->tokens,
                &concat
            );
        }

        symbol.ch = '\\';
        vector_append(
            &scanner->tokens,
            &symbol
        );    
        symbol.ch = curr;
        if (
            curr == '\0'
        ) token.class = EOT;
        
        return symbol;
    }
    }
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
    Token concat = {
        .class  = CONCAT
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

static Token handle_invalid_operation_syntax(
    Scanner *scanner,
    Vector  *symbols,
    char   curr
){
    Token symbol = {
        .class  = SYMBOL
    };
    Token concat = {                                            
        .class  = CONCAT                                     
    };                                                          
    if (                                                        
        scanner->last_token &&                                  
        (                                                       
            is_terminal_token(*scanner->last_token)         ||  
            scanner->last_token->class == CLOSE_PARENTHESES ||  
            is_quantifier_token(*scanner->last_token)           
        )                                                       
    ){                                                          
        vector_append(                                          
            &scanner->tokens,                                   
            &concat                                             
        );                                                      
    }                                                           
    vector_concat(                                              
        &scanner->tokens,                                       
        symbols                                                 
    );                                                          
    if (                                                        
        vector_get_size(scanner->tokens) > 0                    
    ){                                                          
        scanner->last_token = vector_get(                       
            scanner->tokens,                                    
            vector_get_size(scanner->tokens) - 1                
        );                                                      
    }                                                           
    vector_destroy(                                             
        symbols                                                 
    );                                                          
    symbol.ch = curr;                                           
    if (                                                        
        curr == '\0'                                            
    ) symbol.class = EOT;                                     
                                                               
    return symbol;
}
