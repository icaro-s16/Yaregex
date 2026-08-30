#include "yar_parser_tests.c"
#include "yar_dfa.h"

int main(){
    char *pattern = "a?";
    char *text = "axc";
    uint8_t err = 0;
    FSM dfa = yar_dfa_construct(
        pattern,
        &err
    );

    printf(
        "%s\n",
        (
            yar_dfa_match(
                &dfa, 
                text
            )
        ) ? "True" : "False"
    );

    return 0;
}