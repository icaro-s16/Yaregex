#pragma once 

#include "yar_parser.h"

typedef struct{
    uint8_t is_final;
    Vector  *transitions;
}State;

typedef struct{
    uint8_t is_empty;
    Token   symbol;
    State   *dest;
}Transition;

typedef struct{
    State   *initial_state;
    State   *final_state;
}FsmFragment;

typedef struct{
    enum FsmType{
        DFA,
        NFA
    }type;
    union {
        /*
         * NOTE: When a nfa is constructed with 
         * thompsom's construction, the fsm have 
         * just one final state and the FsmFragment
         * can be used.
         */
        struct{
            FsmFragment *fsm_fragment;
            Vector      *alphabet;
        };
        /*
        * NOTE: When an NFA is converted to 
        * a DFA, it can result in more 
        * than one final state, so the 
        * FsmFragment cannot be used.
        */
        State *initial_state;
    };
    Vector *states;
}FSM;

FSM yar_nfa_construct(const char *pattern, uint8_t *err);

void yar_nfa_destroy(FSM *fsm);

static Vector* get_alphabet(Vector *tokens);

static FsmFragment* fsm_fragment_construct(AstNode *root, Vector *states);

static FsmFragment* terminal_fsm_fragment(Token symbol, Vector *states);

static FsmFragment* concat_fsm_fragment(FsmFragment *left, FsmFragment *right, Vector *states);

static FsmFragment* pipe_fsm_fragment(FsmFragment *left, FsmFragment *right, Vector *states);

static FsmFragment* qmark_fsm_fragment(FsmFragment *left, Vector *states);

static FsmFragment* star_fsm_fragment(FsmFragment *left, Vector *states);

static FsmFragment* plus_fsm_fragment(FsmFragment *left, Vector *states);
