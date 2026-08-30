#include "yar_parser_tests.c"
#include "yar_dfa.h"

int main(){
    char *pattern = "a*";
    parser_test(pattern);
    uint8_t err = 0;
    yar_dfa_construct(pattern, &err);
    return 0;
}