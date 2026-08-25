#include "yar_scanner_tests.c"
#include "yar_parser_tests.c"


int main(){
    parser_test(
        "(a|b)[a,]"
    );
    return 0;
}