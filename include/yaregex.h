#pragma once 

#include <stdint.h>
#include "yar_err_flags.h"

struct FSM;

typedef struct{
    uint64_t start, end;    
}YarSlice;

typedef struct{
    struct FSM *dfa;
}YaRegex;

YaRegex* yar_construct(const char *pattern, uint8_t *err);

void yar_destroy(YaRegex *regex);

int yar_match_full(YaRegex *regex, const char *text);

int yar_search(YaRegex *regex, const char *text);

YarSlice yar_capture(YaRegex *regex, char *text);
