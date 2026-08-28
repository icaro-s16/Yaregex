#pragma once

#include "yar_nfa.h"

typedef struct{
    uint8_t     is_final;
    Vector      *states;
    Vector      *transitions;
}SUnion;

typedef struct{
    Token   symbol;
    SUnion  *dest;
}SUTransition;

FSM yar_dfa_construct(const char *pattern, uint8_t *err);

void yar_dfa_destroy(FSM *dfa);

static SUnion sunion_construct(Vector *states);

static SUnion sunion_destroy(SUnion *sunion);

static int sunion_compare(SUnion *su1, SUnion *su2);

/*
 * NOTE: The old NFA will be 
 * destroyed.
 */
static FSM nfa_to_dfa(FSM *nfa);

static void empty_transitions_closure(State *curr, Vector *states);

static void symbol_transitions_closure(const Token symbol, State *curr, Vector *states);

static void set_dfa_final_states(const State *nfa_final_state, Vector *nfa_states);