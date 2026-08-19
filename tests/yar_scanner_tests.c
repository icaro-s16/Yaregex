#include <stdio.h>
#include "yar_scanner.h"

int main(){ 
    Vector *tokens = yar_scan("[a-b]");
    
    if (
        !tokens
    ) {
        fprintf(
            stderr,
            "[ERROR] Invalid token\n"
        );
        exit(EXIT_FAILURE);
    }
    for(
        int i = 0; 
        i < vector_get_size(tokens); 
        i++
    ){
        printf("%s\n", yar_token_to_string(*(Token*)vector_get(tokens, i)));
    }
    vector_destroy(tokens);
    return 0;
}

