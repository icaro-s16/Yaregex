#include "yaregex.h"
#include <stdio.h>

int main(){
    uint8_t err = 0;
    YaRegex *regex = yar_construct(
        "($|[rR]$)?(\\d|[1-9]\\d*)([,\\.]\\d*)?", 
        &err
    );

    if (
        err 
    ) {
        printf("%d\n", err);
        return 1;
    }

    printf("%s\n", (yar_match_full(regex, "r$1230,123"))?"True":"False");


    return 0;
}