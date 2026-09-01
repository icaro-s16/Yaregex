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

static int sunion_compare(SUnion *su1, SUnion *su2);

static Vector* sunions_construct(FSM *nfa, uint8_t *err);

static void sunion_destroy(SUnion *sunion);

static void sunions_destroy(Vector *sunions);

static FSM sunions_to_dfa(Vector *sunions, uint8_t *err);

static int dfa_recursive_conversion(SUnion *curr, SUTransition *curr_transition, Vector *alphabet, Vector *sunions);

static int empty_transitions_closure(State *curr, Vector *sunions);

static int symbol_transitions_closure(const Token symbol, State *curr, Vector *sunions);

static void set_final_sunions(const State *nfa_final_state, Vector *sunions);