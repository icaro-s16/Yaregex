#pragma once 

#include "yar_parser.h"

typedef struct{
    uint    is_final;
    Vector  *transitions;
}State;

typedef struct{
    uint            is_empty;
    Token           symbol;
    State           *dest;
}Transition;

typedef struct{
    State   *initial_state;
    State   *final_state;
}FsmFragment;

typedef struct{
    FsmFragment *fsm_fragment;
    Vector      *states;
}FSM;

FSM yar_nfa_construct(const char *pattern, uint *err);

void yar_nfa_destroy(FSM *fsm);

static FsmFragment* fsm_fragment_construct(AstNode *root, Vector *states);

static FsmFragment* terminal_fsm_fragment(Token symbol, Vector *states);

static FsmFragment* concat_fsm_fragment(FsmFragment *left, FsmFragment *right, Vector *states);

static FsmFragment* pipe_fsm_fragment(FsmFragment *left, FsmFragment *right, Vector *states);

static FsmFragment* qmark_fsm_fragment(FsmFragment *left, Vector *states);

static FsmFragment* star_fsm_fragment(FsmFragment *left, Vector *states);

static FsmFragment* plus_fsm_fragment(FsmFragment *left, Vector *states);
