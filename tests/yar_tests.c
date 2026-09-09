#include "yaregex.h"
#include <stdio.h>

int main(){
    uint8_t err = 0;
    YaRegex *regex = yar_construct(
        "(R|r)$([1-9]|[1-9][0-9]{1,2})(\\.[0-9]{3})*(,[0-9]+)?", 
        &err
    );

    if (
        err 
    ) {
        printf("%d\n", err);
        return 1;
    }

    printf(
        "%d\n",
        yar_match_full(regex, "R$1.254,00")
    );

    printf(
        "%d\n",
        yar_search(regex, "R$12akjk1aaR$123[0,9]")
    );

    char *text = "1213R$2lklaadpalkpsdl1";
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