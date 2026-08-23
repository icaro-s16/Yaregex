#pragma once 

#include "yar_parser.h"

typedef struct{
    uint        is_final;
    Transition  *transitions;
}State;

typedef struct{
    uint    is_empty_transtition;
    Token   symbol;
    State   *dest;
}Transition;






