#include "yar_parser_tests.c"
#include "yar_dfa.h"

int main(){
    char *pattern = "((1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*|(0|1|2|3|4|5|6|7|8|9))(,(0|1|2|3|4|5|6|7|8|9)+)?";
    char *text = "13837873,787878";
    uint8_t err = 0;
    FSM dfa = yar_dfa_construct(
        pattern,
        &err
    );

    if (
        err 
    ) printf("Erro\n");
    else {
        printf(
            "%s\n",
            (
                yar_dfa_match(
                    &dfa, 
                    text
                )
            ) ? "True" : "False"
        );
        yar_dfa_destroy(
            &dfa
        );
    }
    return 0;
}