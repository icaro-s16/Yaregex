#include "yar_scanner_tests.c"
#include "yar_parser_tests.c"
#include "yar_nfa_tests.c"


int main(){
    parser_test(
        "((maria)*|(icaro)*)*|(icaro e maria)"
    );
    return 0;
}