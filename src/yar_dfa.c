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
    assert(
        pattern && 
        err
    );

    if (
        !strlen(
            pattern
        ) 
    ) {
        *err |= YAR_INVALID_PATTERN;
        return (FSM){0};
    }

    FSM nfa = yar_nfa_construct(
        pattern,
        err
    );

    if (
        *err 
    ) return (FSM){0};

    Vector *sunions = sunions_construct(
        &nfa,
        err
    );

    FSM dfa = sunions_to_dfa(
        sunions,
        err
    );

    sunions_destroy(
        sunions
    );

    yar_nfa_destroy(
        &nfa
    );

    return dfa;
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

        if (
            (*curr)->transitions
        ){
            vector_destroy(
                (*curr)->transitions
            );
        }

        if (
            *curr 
        ){
            free(
                *curr
            );
        }
    }

    assert(
        !vector_destroy(
            dfa->states
        )
    );
}

static Vector* sunions_construct(
    FSM     *nfa,
    uint8_t *err
){
    int vec_err = 0;

    Vector *sunions = vector_construct(
        sizeof(SUnion*)
    );

    if (
        !sunions
    ) goto invalid_alloc;

    SUnion *initial_sunion = calloc(
        1,
        sizeof(SUnion)
    );

    if (
        !initial_sunion
    ) goto invalid_alloc;

    vec_err = vector_append(
        &sunions,
        &initial_sunion,
        sizeof(SUnion*)
    );

    if (
        vec_err 
    ) goto invalid_vec_alloc;
    

    initial_sunion->states = vector_construct(
        sizeof(State*)
    );

    if (
        !initial_sunion->states
    ) goto invalid_alloc;

    vec_err = vector_append(
        &initial_sunion->states,
        &nfa->fsm_fragment->initial_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    if (
        empty_transitions_closure(
            nfa->fsm_fragment->initial_state,
            initial_sunion->states
        )
    ) goto invalid_alloc; 

    initial_sunion->transitions = vector_construct(
        sizeof(SUTransition)
    );

    if (
        !initial_sunion->transitions
    ) goto invalid_alloc;

    for(
        int symbol_idx = 0;
        symbol_idx < vector_get_size(
            nfa->alphabet
        );
        symbol_idx++
    ){
        Token *symbol = vector_get(
            nfa->alphabet,
            symbol_idx
        );

        SUTransition transition = {
            .dest   = calloc(
                1,
                sizeof(SUnion)
            ),
            .symbol = *symbol
        };

        if (
            !transition.dest
        ) goto invalid_alloc;
        
        transition.dest->states = vector_construct(
            sizeof(State*)
        );

        if (
            !transition.dest->states
        ) {
            free(
                transition.dest
            );

            goto invalid_alloc;
        }

        for(
            int sunion_idx = 0;
            sunion_idx < vector_get_size(
                initial_sunion->states
            );
            sunion_idx++
        ){
            State **curr_s = vector_get(
                initial_sunion->states,
                sunion_idx
            );

            if (
                symbol_transitions_closure(
                    *symbol,
                    *curr_s,
                    transition.dest->states
                )
            ) goto invalid_alloc;
        }

        if (
            !vector_get_size(
                transition.dest->states
            )
        ){
            sunion_destroy(
                transition.dest
            );

            free(
                transition.dest
            );

            continue;
        }

        if(
            dfa_recursive_conversion(
                initial_sunion,
                &transition,
                nfa->alphabet,
                sunions
            )
        ) goto invalid_alloc;
        

        vec_err = vector_append(
            &initial_sunion->transitions,
            &transition,
            sizeof(Transition)
        );

        if (
            vec_err
        ) goto invalid_vec_alloc;
        
    }   

    set_final_sunions(
        nfa->fsm_fragment->final_state,
        sunions
    );

    return sunions;
    
    invalid_vec_alloc:
    
        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:

        if (
            sunions
        ) {
            sunions_destroy(
                sunions
            );

            assert(
                !vector_destroy(
                    sunions
                )
            );
        }

        *err |= YAR_INVALID_ALLOC;
        return NULL;
}


static void sunion_destroy(
    SUnion *sunion
){
    assert(
        sunion          &&
        sunion->states
    );


    if (
        sunion->states
    ){
        vector_destroy(
            sunion->states
        );
    }

    if (
        sunion->transitions
    ) {  
        vector_destroy(
            sunion->transitions
        );
    }
}

static void sunions_destroy(
    Vector *sunions
){
    for(
        int idx = 0;
        idx < vector_get_size(
            sunions
        );
        idx++
    ){
        SUnion **curr = vector_get(
            sunions,
            idx
        );

        sunion_destroy(
            *curr
        );

        free(
            *curr
        );

        *curr = NULL;
    }

    vector_destroy(
        sunions
    );
}

static FSM sunions_to_dfa(
    Vector  *sunions,
    uint8_t *err
){
    int vec_err = 0;

    Vector *states = vector_construct(
        sizeof(State*)
    );

    if (
        !states 
    ) goto invalid_alloc;

    for(
        int idx = 0;
        idx < vector_get_size(
            sunions
        );
        idx ++
    ) {
        State *curr = calloc(
            1,
            sizeof(State)
        );

        if (
            !curr 
        ) goto invalid_alloc;

        vec_err = vector_append(
            &states,
            &curr,
            sizeof(State*)
        );

        if (
            vec_err
        ) goto invalid_vec_alloc;
    }

    for(
        int idx = 0;
        idx < vector_get_size(
            sunions
        );
        idx ++
    ) {

        State **curr_state = vector_get(
            states,
            idx
        );

        SUnion **curr_sunion = vector_get(
            sunions,
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

        if (
            !(*curr_state)->transitions 
        ) goto invalid_alloc;

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
                states,
                vector_find(
                    sunions,
                    &curr_sutranstion->dest,
                    sizeof(SUnion*)
                )
            ));

            vec_err = vector_append(
                &(*curr_state)->transitions,
                &curr_transition,
                sizeof(Transition)
            );

            if (
                vec_err 
            ) goto invalid_vec_alloc;

        }
        
    }

    return (FSM) {
        .type           = DFA,
        .initial_state  = *((State**)vector_get(
            states,
            0
        )),
        .states         = states,
    };

        
    invalid_vec_alloc:
    
        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:

        if (
            states
        ) {

            for(
                int idx = 0;
                idx < vector_get_size(
                    states
                );
                idx++  
            ){
                State **state = vector_get(
                    states,
                    idx
                );

                vector_destroy(
                    (*state)->transitions
                );

                free(
                    *state
                );
            }

            assert(
                !vector_destroy(
                    states
                )
            );
        }

        *err |= YAR_INVALID_ALLOC;
        return (FSM){0};
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

static SUnion* dfa_create_initial_state(
    FSM     *nfa,
    Vector  *sunions
){

}

static int dfa_recursive_conversion(
    SUnion          *curr,
    SUTransition    *curr_transition, 
    Vector          *alphabet, 
    Vector          *sunions
){
    assert(
        alphabet        &&
        sunions    
    );

    int err = 0;
    
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
        if (
            empty_transitions_closure(
                *curr_s,
                curr_transition->dest->states
            )
        ) {
            sunion_destroy(
                curr_transition->dest
            );

            free(
                curr_transition->dest
            );

            return YAR_INVALID_ALLOC;
        }
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
        sunion_destroy(
            curr_transition->dest
        );

        free(
            curr_transition->dest
        );

        curr_transition->dest = *loop_state;
        return 0;
    }
    else{
        err = vector_append(
            &sunions,
            &curr_transition->dest,
            sizeof(SUnion*)
        );

        if (
            err 
        ) {
            assert(
                err == VEC_RESIZE_FAILED
            );

            sunion_destroy(
                curr_transition->dest
            );

            free(
                curr_transition->dest
            );
            
            return YAR_INVALID_ALLOC;
        }
    }

    curr_transition->dest->transitions = vector_construct(
        sizeof(SUTransition)
    );

    if (
        !curr_transition->dest->transitions
    ) return YAR_INVALID_ALLOC;

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

        if (
            !transition.dest
        ) return YAR_INVALID_ALLOC;

        transition.dest->states = vector_construct(
            sizeof(State*)
        );

        if (
           !transition.dest->states
        ) {
            free(
                transition.dest
            );

            return YAR_INVALID_ALLOC;
        }
        
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

            if (
                symbol_transitions_closure(
                    *symbol,
                    *curr_s,
                    transition.dest->states
                )
            ) {
                sunion_destroy(
                    transition.dest
                );

                free(
                    transition.dest
                );

                return YAR_INVALID_ALLOC;
            };
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

        if (
            dfa_recursive_conversion(
                curr_transition->dest,
                &transition,
                alphabet,
                sunions
            )
        ) return YAR_INVALID_ALLOC;

        err = vector_append(
            &curr_transition->dest->transitions,
            &transition,
            sizeof(Transition)
        );

        if (
            err
        ) {
            assert(
                err == VEC_RESIZE_FAILED
            );

            return YAR_INVALID_ALLOC;
        }
    }

    return 0;
}


static int empty_transitions_closure(
    State   *curr_s, 
    Vector  *sunions
){
    assert(
        curr_s  && 
        sunions
    );

    int err = 0;

    if (
        !curr_s->transitions
    ) return 0;

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
                sunions,
                &curr_t->dest,
                sizeof(State*)
            ) >= 0
        ) continue;

        err = vector_append(
            &sunions,
            &curr_t->dest,
            sizeof(State*)
        );

        if (
            err 
        ) goto invalid_vec_alloc;

        if (
            empty_transitions_closure(
                curr_t->dest,
                sunions
            )
        ) return YAR_INVALID_ALLOC;
    }

    return 0;

    invalid_vec_alloc:

        assert(
            err == VEC_RESIZE_FAILED
        );

        return YAR_INVALID_ALLOC;
}

static int symbol_transitions_closure(
    const Token symbol, 
    State       *curr_s, 
    Vector      *sunions
){
    assert(
        is_terminal_token(symbol) &&
        curr_s                    &&
        sunions
    );

    int err = 0;

    if (
        !curr_s->transitions
    ) return 0;

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
                sunions,
                &curr_t->dest,
                sizeof(State*)
            ) >= 0
        ) continue;

        err = vector_append(
            &sunions,
            &curr_t->dest,
            sizeof(State*)
        );

        if (
            err 
        ) goto invalid_vec_alloc;
    }

    return 0;

    invalid_vec_alloc:

        assert(
            err == VEC_RESIZE_FAILED
        );

        return YAR_INVALID_ALLOC;
}

static void set_final_sunions(
    const State *nfa_final_state, 
    Vector      *sunions
){
    assert(
        nfa_final_state             &&
        sunions                 &&
        vector_get_data_type_size(
            sunions
        ) == sizeof(SUnion*)
    );

    for(
        int idx = 0;
        idx < vector_get_size(
            sunions
        );
        idx++
    ){
        SUnion **curr = vector_get(
            sunions,
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