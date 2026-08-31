#include "yar_dfa.h"
#include <stdio.h>

int yar_dfa_match(
    FSM *dfa,
    const char *text
){
    assert(
        dfa &&
        dfa->type == DFA
    );

    State *curr_state = dfa->initial_state;

    for(
        int text_idx = 0;
        text_idx < strlen(text);
        text_idx++
    ){
        int valid_transition = 0;

        if (
            !curr_state->transitions
        ) return 0;

        for (
            int transition_idx = 0;
            transition_idx < vector_get_size(
                curr_state->transitions
            );
            transition_idx++
        ) {
            Transition *curr_transition = vector_get(
                curr_state->transitions,
                transition_idx
            );

            if (
                curr_transition->symbol.ch != text[text_idx]
            ) continue;

            valid_transition = 1;
            curr_state = curr_transition->dest;

            break;
        }

        if (
            !valid_transition
        ) return 0;

    }

    return curr_state->is_final;
}

FSM yar_dfa_construct(
    const char  *pattern, 
    uint8_t     *err
){
    FSM nfa = yar_nfa_construct(
        pattern,
        err
    );

    if (
        *err 
    ) return (FSM){0};
    

    Vector *dfa_sunions = vector_construct(
        sizeof(SUnion*)
    );

    SUnion *initial_state = calloc(
        1,
        sizeof(SUnion)
    );

    vector_append(
        &dfa_sunions,
        &initial_state,
        sizeof(SUnion*)
    );

    initial_state->states = vector_construct(
        sizeof(State*)
    );

    vector_append(
        &initial_state->states,
        &nfa.fsm_fragment->initial_state,
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
            int sunion_idx = 0;
            sunion_idx < vector_get_size(
                initial_state->states
            );
            sunion_idx++
        ){
            State **curr_s = vector_get(
                initial_state->states,
                sunion_idx
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

        dfa_recursive_conversion(
            initial_state,
            &transition,
            nfa.alphabet,
            dfa_sunions
        );

        vector_append(
            &initial_state->transitions,
            &transition,
            sizeof(Transition)
        );
    }   

    set_dfa_final_states(
        nfa.fsm_fragment->final_state,
        dfa_sunions
    );

    Vector *dfa_states = vector_construct(
        sizeof(State*)
    );

    for(
        int idx = 0;
        idx < vector_get_size(
            dfa_sunions
        );
        idx ++
    ) {
        State *curr = calloc(
            1,
            sizeof(State)
        );

        vector_append(
            &dfa_states,
            &curr,
            sizeof(State*)
        );
    }

    for(
        int idx = 0;
        idx < vector_get_size(
            dfa_sunions
        );
        idx ++
    ) {

        State **curr_state = vector_get(
            dfa_states,
            idx
        );

        SUnion **curr_sunion = vector_get(
            dfa_sunions,
            idx
        );

        (*curr_state)->is_final = (*curr_sunion)->is_final;

        if (
            !vector_get_size(
                (*curr_sunion)->transitions
            )
        ) continue;

        Transition curr_transition = {
            .is_empty = 0
        };

        (*curr_state)->transitions = vector_construct(
            sizeof(Transition)
        );

        for(
            int sutransition_idx = 0;
            sutransition_idx < vector_get_size(
                (*curr_sunion)->transitions
            );
            sutransition_idx ++
        ){
            SUTransition *curr_sutranstion = vector_get(
                (*curr_sunion)->transitions,
                sutransition_idx
            );

            curr_transition.symbol = curr_sutranstion->symbol;

            curr_transition.dest = *((State**)vector_get(
                dfa_states,
                vector_find(
                    dfa_sunions,
                    &curr_sutranstion->dest,
                    sizeof(SUnion*)
                )
            ));

            vector_append(
                &(*curr_state)->transitions,
                &curr_transition,
                sizeof(Transition)
            );

        }
        
    }

    for(
        int idx = 0;
        idx < vector_get_size(
            dfa_sunions
        );
        idx++
    ){
        SUnion **curr = vector_get(
            dfa_sunions,
            idx
        );

        sunion_destroy(
            *curr
        );
    }

    vector_destroy(
        dfa_sunions
    );

    yar_nfa_destroy(
        &nfa
    );

    return (FSM) {
        .type           = DFA,
        .initial_state  = *((State**)vector_get(
            dfa_states,
            0
        )),
        .states         = dfa_states,
    };
}

void yar_dfa_destroy(
    FSM *dfa
){
    assert(
        dfa                 &&
        dfa->type == DFA    &&
        dfa->initial_state
    );

    for(
        int idx = 0;
        idx < vector_get_size(
            dfa->states
        );
        idx ++
    ){
        State **curr = vector_get(
            dfa->states,
            idx
        );

        vector_destroy(
            (*curr)->transitions
        );

        free(
            *curr
        );

    }
}


static SUnion sunion_destroy(
    SUnion *sunion
){
    assert(
        sunion          &&
        sunion->states
    );

    vector_destroy(
        sunion->states
    );

    if (
        sunion->transitions
    ) {  
        vector_destroy(
            sunion->transitions
        );
    }
}

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

static void dfa_recursive_conversion(
    SUnion          *curr,
    SUTransition    *curr_transition, 
    Vector          *alphabet, 
    Vector          *sunions
){
    assert(
        alphabet        &&
        sunions    
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

    int is_self_loop = 0;
    SUnion **loop_state = NULL;
    for(
        int idx = 0;
        idx < vector_get_size(
            sunions
        );
        idx++
    ){
        loop_state = vector_get(
            sunions,
            idx
        );

        if (
            sunion_compare(
                *loop_state,
                curr_transition->dest
            )
        ) {
            is_self_loop = 1;
            break;
        }
    }

    if (
        is_self_loop
    ) {
        vector_destroy(
            curr_transition->dest->states
        );
        free(
            curr_transition->dest
        );
        curr_transition->dest = *loop_state;
        return;
    }
    else{
        vector_append(
            &sunions,
            &curr_transition->dest,
            sizeof(SUnion*)
        );
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

        dfa_recursive_conversion(
            curr_transition->dest,
            &transition,
            alphabet,
            sunions
        );

        vector_append(
            &curr_transition->dest->transitions,
            &transition,
            sizeof(Transition)
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
            &curr_t->dest,
            sizeof(State*)
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
            curr_t->is_empty                    ||
            curr_t->symbol.ch != symbol.ch      
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
            &curr_t->dest,
            sizeof(State*)
        );
    }
}

static void set_dfa_final_states(
    const State *nfa_final_state, 
    Vector      *dfa_sunions
){
    assert(
        nfa_final_state             &&
        dfa_sunions                 &&
        vector_get_data_type_size(
            dfa_sunions
        ) == sizeof(SUnion*)
    );

    for(
        int idx = 0;
        idx < vector_get_size(
            dfa_sunions
        );
        idx++
    ){
        SUnion **curr = vector_get(
            dfa_sunions,
            idx
        );

        if (
            vector_find(
                (*curr)->states,
                &nfa_final_state,
                sizeof(State*)
            ) < 0
        ) continue;

        (*curr)->is_final = 1;
    }
}