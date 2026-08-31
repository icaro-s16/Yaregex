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

int yar_dfa_match(FSM *dfa, const char *text);

FSM yar_dfa_construct(const char *pattern, uint8_t *err);

void yar_dfa_destroy(FSM *dfa);

static SUnion sunion_destroy(SUnion *sunion);

static int sunion_compare(SUnion *su1, SUnion *su2);

static void dfa_recursive_conversion(SUnion *curr, SUTransition *curr_transition, Vector *alphabet, Vector *sunions);

static void empty_transitions_closure(State *curr, Vector *states);

static void symbol_transitions_closure(const Token symbol, State *curr, Vector *states);

static void set_dfa_final_states(const State *nfa_final_state, Vector *nfa_states);