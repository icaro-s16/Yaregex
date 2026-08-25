#include <stdio.h>
#include "yar_parser.h"
#include "yar_scanner.h"

void parser_test(
    char *pattern
){ 
    Vector *tokens = yar_scan(pattern);
    uint err = 0;
    ASTNode *root = parser_create_ast(tokens, &err);
    if (
        err != 0
    ){
        if (
            err & INVALID_QUANTIFIER
        ) printf("Invalid quantifier\n");
        if(
            err & INVALID_ALTERNATION 
        ) printf("Invalid alternation\n");
        if (
            err & INVALID_GROUPING
        ) printf("Invalid grouping\n");
    }
    else {
        parser_print_ast(root);
        parser_destroy_ast(root);
    }
}