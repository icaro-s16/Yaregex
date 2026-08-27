#include "yar_scanner_tests.c"
#include "yar_parser_tests.c"
#include "yar_nfa_tests.c"


int main(){
    scanner_test(
        "a()"
    );
    parser_test(
        "a()"
    );
    return 0;
}