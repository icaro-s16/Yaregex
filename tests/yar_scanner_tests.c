#include "../src/yar_scanner.c"

const char *tokens_name[] = {
    [T_OPARENTHESES] = "OPARENTHESES",
    [T_CPARENTHESES] = "CPARENTHESES",
    [T_CIRCUMFLEX]   = "CIRCUMFLEX",
    [T_BACKSLASH]    = "BACKSLASH",
    [T_OBRACKET]     = "OBRACKET",
    [T_CBRACKET]     = "CBRACKET",
    [T_MINUSS]       = "MINUSS",
    [T_DOLARS]       = "DORLARS",
    [T_PLUSS]        = "PLUSS",
    [T_QMARK]        = "QMARK",
    [T_STAR]         = "STAR",
    [T_PIPE]         = "PIPE",
    [T_DOT]          = "DOT",  
    [T_EOF]          = "EOF",
    [T_CHAR]         = "CHAR"    
};

int main(){
    Vector *tokens = yar_scan("(ab|ar)?+ab*");
    for(int i = 0; i < vector_get_size(tokens); i++){
        printf("%s [%c] ", tokens_name[((Token*)vector_get(tokens, i))->type], ((Token*)vector_get(tokens, i))->val);
    }
    vector_destroy(tokens);
    return 0;
}

