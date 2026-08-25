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
    State *initial_state;
    State *final_state;
}FSM;

FSM* yar_create_automaton(ASTNode *root);

static FSM* terminal_fsm(Token symbol);

static FSM* concat_fsm(FSM *left, FSM *right);

static FSM* pipe_fsm(FSM *left, FSM *right);

static FSM* qmark_fsm(FSM *left, FSM *right);

static FSM* star_fsm(FSM *left, FSM *rihgt);

static FSM* plus_fsm(FSM *left, FSM *right);
