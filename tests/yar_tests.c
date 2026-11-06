#include "yaregex.h"
#include <stdio.h>

int main(){
    uint8_t err = 0;
    YaRegex *regex = yar_construct(
        "", 
        &err
    );

    if (
        err 
    ) {
        printf("%d\n", err);
        return 1;
    }

    printf("%d\n", yar_match_full(regex, "0.99"));


    return 0;
}