#include "yaregex.h"
#include "yar_dfa.h"

YaRegex* yar_construct(
    const char  *pattern,
    uint8_t     *err
){
    FSM *dfa = NULL;
    YaRegex *regex = NULL;

    regex = calloc(
        1,
        sizeof(
            YaRegex
        )
    );

    if (
        !regex
    ) {
        *err |= YAR_INVALID_ALLOC;
        goto invalid_alloc;
    }

    dfa = calloc(
        1,
        sizeof(
            FSM
        )
    );

    if (
        !dfa 
    ) {
        *err |= YAR_INVALID_ALLOC;
        goto invalid_alloc;
    };

    FSM fsm = yar_dfa_construct(
        pattern,
        err
    );

    if (
        *err 
    ) goto invalid_alloc;

    dfa->type = fsm.type;
    dfa->states = fsm.states;
    dfa->initial_state = fsm.initial_state;

    regex->dfa = (struct FSM*)dfa;

    return regex;

    invalid_alloc:

    if (
        regex
    ) free(regex);
    
    if (
        dfa 
    ) free(dfa);

    return NULL;
}

void yar_destroy(
    YaRegex *regex
){
    assert(
        regex &&
        regex->dfa
    );

    yar_dfa_destroy(
        (FSM*)regex->dfa
    );

    free(
        regex->dfa
    );

    free(
        regex
    );
}

int yar_match_full(
    YaRegex *regex, 
    const char *text
){
    assert(
        regex->dfa
    );

    State *curr_state = ((FSM*)regex->dfa)->initial_state;

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
                curr_transition->symbol != text[text_idx]
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

int yar_search(
    YaRegex     *regex, 
    const char  *text
){
    assert(
        regex->dfa &&
        text
    );

    State *curr_state = ((FSM*)regex->dfa)->initial_state;

    for(
        int text_idx = 0;
        text_idx < strlen(text);
        text_idx++
    ){
        int valid_transition = 0;

        if (
            !curr_state->transitions
        ) break;

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
                curr_transition->symbol != text[text_idx]
            ) continue;
            
            valid_transition = 1;
            curr_state = curr_transition->dest;
            break;
        }

        if (
            !valid_transition &&
            curr_state->is_final
        ) break;

    }

    return curr_state->is_final;
}


YarSlice yar_capture(
    YaRegex     *regex, 
    char        *text
){
    assert(
        regex->dfa
    );

    static char *saved_text = NULL;
    static size_t saved_idx = 0;

    if (
        text 
    ){
        saved_text = text;
        saved_idx = 0;
    }

    int text_idx = saved_idx;
    int first_valid_idx = 1;
    YarSlice slice = {0};

    State *curr_state = ((FSM*)regex->dfa)->initial_state;

    if (
        !saved_text
    ) return (YarSlice){0};

    if (
        saved_text[saved_idx] == '\0'
    ) return (YarSlice){0};

    for(
        ;
        text_idx < strlen(saved_text);
        text_idx++
    ){
        int valid_transition = 0;

        if (
            !curr_state->transitions &&
            curr_state->is_final
        ) break;

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
                curr_transition->symbol != saved_text[text_idx]
            ) continue;
            
            if (
                first_valid_idx
            ) {
                slice.start = text_idx;
                first_valid_idx = 0;
            }
            
            valid_transition = 1;
            curr_state = curr_transition->dest;

            break;
        }

        if (
            !valid_transition 
        ) {
            if (
                curr_state->is_final && 
                !first_valid_idx
            ) break;

            first_valid_idx = 1;
        }
    }
    
    if (
        !first_valid_idx &&
        curr_state->is_final
    ){
        slice.end = text_idx - 1;
        saved_idx = text_idx;
    }
    else{
        slice.start = 0;
    }

    return slice;
}