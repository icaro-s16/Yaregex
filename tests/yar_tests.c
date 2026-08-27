#include "yar_scanner_tests.c"
#include "yar_parser_tests.c"
#include "yar_nfa_tests.c"


int main(){
    parser_test(
        ".@([a-Z])+(.[a-Z]+)?"
    );
    parser_test(
        "(a(((((a))))))"
    );
    return 0;
}