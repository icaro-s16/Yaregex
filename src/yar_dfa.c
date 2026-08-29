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

static SUnion* nfa_to_dfa(
    State   *curr, 
    Vector  *alphabet,
    Vector  *valid_states, 
    Vector  *invalid_states

){
    assert(
        curr            &&
        alphabet        &&
        valid_states    &&
        invalid_states
    );

    SUnion *sunion = calloc(
        1,
        sizeof(SUnion)
    );

    sunion->states = vector_construct(
        sizeof(State*)
    );

    empty_transitions_closure(
        curr,
        sunion->states
    );

    Vector *transitions = calloc(
        1,
        sizeof(SUTransition)
    );

    for (
        int idx_d = 0;
        idx_d < vector_get_size(
            alphabet
        );
        idx_d++
    ){
        SUTransition transition;

        Token *symbol = vector_get(
            alphabet,
            idx_d
        );

        transition.symbol = *symbol;

        SUnion *dest = calloc(
            1,
            sizeof(SUnion)
        );

        dest->states = vector_construct(
            sizeof(State*)
        );

        for(
            int idx_s = 0;
            idx_s < vector_get_size(
                dest->states
            );
            idx_s ++
        ){
            State **curr_s = vector_get(
                dest->states, 
                idx_s
            );

            symbol_transitions_closure(
                *symbol,
                curr_s,
                dest->states
            );
            
        }

        if (
            vector_get_size(
                dest->states
            ) <= 0 
        ) {
            vector_destroy(
                dest->states
            );
            free(
                dest
            );
            continue;
        }
        
        transition.dest = dest;

        vector_append(
            &transitions,
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