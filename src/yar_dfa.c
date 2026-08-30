#include "yar_dfa.h"
#include <stdio.h>

FSM yar_dfa_construct(
    const char  *pattern, 
    uint8_t     *err
){
    FSM nfa = yar_nfa_construct(
        pattern,
        err
    );

    Vector *dfa_states = vector_construct(
        sizeof(SUnion*)
    );

    SUnion *initial_state = calloc(
        1,
        sizeof(SUnion)
    );

    vector_append(
        &dfa_states,
        &initial_state
    );

    initial_state->states = vector_construct(
        sizeof(State*)
    );

    empty_transitions_closure(
        nfa.fsm_fragment->initial_state,
        initial_state->states
    );

    initial_state->transitions = vector_construct(
        sizeof(SUTransition)
    );

    for(
        int symbol_idx = 0;
        symbol_idx < vector_get_size(
            nfa.alphabet
        );
        symbol_idx++
    ){
        Token *symbol = vector_get(
            nfa.alphabet,
            symbol_idx
        );

        SUTransition transition = {
            .dest   = calloc(
                1,
                sizeof(SUnion)
            ),
            .symbol = *symbol
        };

        transition.dest->states = vector_construct(
            sizeof(State*)
        );
        
        for(
            int state_idx = 0;
            state_idx < vector_get_size(
                initial_state->states
            );
            state_idx++
        ){
            State **curr_s = vector_get(
                initial_state->states,
                state_idx
            );

            symbol_transitions_closure(
                *symbol,
                *curr_s,
                transition.dest->states
            );
        }

        if (
            !vector_get_size(
                transition.dest->states
            )
        ){
            vector_destroy(
                transition.dest->states
            );
            free(
                transition.dest
            );
            continue;
        }

        nfa_to_dfa(
            initial_state,
            &transition,
            nfa.alphabet,
            dfa_states
        );

        if (
            !sunion_compare(
                initial_state,
                transition.dest
            )
        ) {
            vector_append(
                &dfa_states,
                &transition.dest
            );
        }

        vector_append(
            &initial_state->transitions,
            &transition
        );
    }   

    printf("Estados: %d\n", vector_get_size(dfa_states));
}

void yar_dfa_destroy(
    FSM *dfa
);

static SUnion sunion_construct(
    Vector *states
);

static SUnion sunion_destroy(
    SUnion *sunion
);

static int sunion_compare(
    SUnion *su1, 
    SUnion *su2
){
    assert(
        su1 &&
        su2
    );

    if (
        vector_get_size(
            su1->states
        ) != 
        vector_get_size(
            su2->states
        )
    ) return 0;

    for(
        int idx = 0;
        idx < vector_get_size(
            su1->states
        );
        idx++
    ){
        State **c1 = vector_get(
            su1->states,
            idx
        );

        State **c2 = vector_get(
            su2->states,
            idx
        );

        if (
            *c1 != *c2 
        ) return 0;
    }

    return 1;
}

static void nfa_to_dfa(
    SUnion          *curr,
    SUTransition    *curr_transition, 
    Vector          *alphabet, 
    Vector          *states
){
    assert(
        alphabet        &&
        states    
    );

    for(
        int idx = 0;
        idx < vector_get_size(
            curr_transition->dest->states
        );
        idx ++
    ){
        State **curr_s = vector_get(
            curr_transition->dest->states,
            idx
        );
        empty_transitions_closure(
            *curr_s,
            curr_transition->dest->states
        );
    }

    if (
        sunion_compare(
            curr,
            curr_transition->dest
        )
    ) {
        vector_destroy(
            curr_transition->dest->states
        );
        free(
            curr_transition->dest
        );
        curr_transition->dest = curr;
        return;
    }

    curr_transition->dest->transitions = vector_construct(
        sizeof(SUTransition)
    );

    for(
        int symbol_idx = 0;
        symbol_idx < vector_get_size(
            alphabet
        );
        symbol_idx++
    ){
        Token *symbol = vector_get(
            alphabet,
            symbol_idx
        );

        SUTransition transition = {
            .dest   = calloc(
                1,
                sizeof(SUnion)
            ),
            .symbol = *symbol
        };

        transition.dest->states = vector_construct(
            sizeof(State*)
        );
        
        for(
            int state_idx = 0;
            state_idx < vector_get_size(
                curr_transition->dest->states
            );
            state_idx++
        ){
            State **curr_s = vector_get(
                curr_transition->dest->states,
                state_idx
            );

            symbol_transitions_closure(
                *symbol,
                *curr_s,
                transition.dest->states
            );
        }

        if (
            !vector_get_size(
                transition.dest->states
            )
        ){
            vector_destroy(
                transition.dest->states
            );
            free(
                transition.dest
            );
            continue;
        }

        nfa_to_dfa(
            curr_transition->dest,
            &transition,
            alphabet,
            states
        );

        if (
            !sunion_compare(
                curr_transition->dest,
                transition.dest
            )
        ) {
            vector_append(
                &states,
                &transition.dest
            );
        }

        vector_append(
            &curr_transition->dest->transitions,
            &transition
        );
    }
}


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