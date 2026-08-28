#include "yar_dfa.h"

FSM yar_dfa_construct(
    const char  *pattern, 
    uint8_t     *err
);

void yar_dfa_destroy(
    FSM *dfa
);

static SUnion sunion_construct(
    Vector *states
);

static SUnion sunion_destroy(
    SUnion *sunion
);

/*
 * NOTE: The old NFA will be 
 * destroyed.
 */
static FSM nfa_to_dfa(
    FSM *nfa
);

static void empty_transitions_closure(
    State   *curr_s, 
    Vector  *states
){
    assert(
        curr_s  && 
        states
    );

    if (
        !curr_s->transitions
    ) return;

    for(
        int idx = 0;
        idx < vector_get_size(
            curr_s->transitions
        );
        idx++
    ){
        Transition *curr_t = vector_get(
            curr_s->transitions,
            idx
        );

        if (
            !curr_t->is_empty ||
            !curr_t->dest
        ) continue;

        if (
            vector_find(
                states,
                &curr_t->dest,
                sizeof(State*)
            ) >= 0
        ) continue;

        vector_append(
            &states,
            &curr_t->dest
        );

        empty_transitions_closure(
            curr_t->dest,
            states
        );
    }
}

static void symbol_transitions_closure(
    const Token symbol, 
    State       *curr_s, 
    Vector      *states
){
    assert(
        is_terminal_token(symbol) &&
        curr_s                    &&
        states
    );

    if (
        !curr_s->transitions
    ) return;

    for(
        int idx = 0;
        idx < vector_get_size(
            curr_s->transitions
        );
        idx++
    ){
        Transition *curr_t = vector_get(
            curr_s->transitions,
            idx
        );

        if (
            (
                curr_t->symbol.ch != symbol.ch
            ) ||
            !curr_t->dest
        ) continue;

        if (
            vector_find(
                states,
                &curr_t->dest,
                sizeof(State*)
            ) >= 0
        ) continue;

        vector_append(
            &states,
            &curr_t->dest
        );

        symbol_transitions_closure(
            symbol,
            curr_t->dest,
            states
        );
    }
}

static void set_dfa_final_states(
    const State *nfa_final_state, 
    Vector      *nfa_states
);