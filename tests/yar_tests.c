#include "yar_scanner_tests.c"
#include "yar_parser_tests.c"
#include "yar_nfa_tests.c"

int main(){
    uint err = 0;
    FSM nfa = yar_nfa_construct("a}", &err);
    printf("%d\n", vector_get_size(nfa.states));
    yar_nfa_destroy(&nfa);
    return 0;
}