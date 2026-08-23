#include <stdio.h>
#include "yar_parser.h"
#include "yar_scanner.h"

int main(){ 
    Vector *tokens = yar_scan("(a|b)*abb");
    for(
        int idx = 0;
        idx < vector_get_size(tokens);
        idx++
    ) printf("%s\n", yar_token_to_string(*((Token*)vector_get(tokens, idx))));
    uint err = 0;
    ASTNode *root = parser_create_ast(tokens, &err);
    if (
        err != 0
    ){
        printf("error\n");
    }
    else {
        parser_print_ast(root);
        parser_destroy_ast(root);
    }
    return 0;
}