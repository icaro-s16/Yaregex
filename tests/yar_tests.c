#include "yaregex.h"
#include <stdio.h>

int main(){
    uint8_t err = 0;
    YaRegex *regex = yar_construct(
        "[0-9]*", 
        &err
    );

    printf(
        "%d\n",
        yar_match_full(regex, "akjkajdkajdk")
    );

    printf(
        "%d\n",
        yar_search(regex, "akjk1aajdk[0,9]")
    );

    char *text = "12132lklaadpalkpsdl1";
    YarSlice slice = yar_capture(regex, text);

    printf(
        "%d-%d\n",
        slice.start, slice.end
    );

    slice = yar_capture(regex, NULL);

    printf(
        "%d-%d\n",
        slice.start, slice.end
    );


    yar_destroy(regex);

    return 0;
}