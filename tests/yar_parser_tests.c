#include <stdio.h>
#include "yar_parser.h"
#include "yar_scanner.h"

void parser_test(
    char *pattern
){ 
    uint8_t err = 0;
    Vector *tokens = yar_scan(pattern, &err);

    if (
        err 
    ){
        vector_destroy(
            tokens
        );

        return;
    }

    AstNode *root = yar_ast_construct(
        tokens, 
        &err
    );
    
    if (
        err != 0
    ){
        if (
            err & PARSER_INVALID_QUANTIFIER
        ) printf("Invalid quantifier\n");
        if(
            err & PARSER_INVALID_ALTERNATION 
        ) printf("Invalid alternation\n");
        if (
            err & PARSER_INVALID_GROUPING
        ) printf("Invalid grouping\n");
    }
    else {
        yar_print_ast(root);
        yar_ast_destroy(root);
    }
    
}