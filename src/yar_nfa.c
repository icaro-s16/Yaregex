#include "yar_nfa.h"


FSM yar_nfa_construct(
    const char  *pattern,
    uint8_t     *err
){
    assert(
        pattern &&
        err
    );

    FsmFragment *fsm_fragment = NULL;

    Vector *states = NULL;

    AstNode *root = NULL;
    
    Vector *tokens = yar_scan(
        pattern,
        err
    );

    if (
        *err 
    ) goto invalid_alloc;

    states = vector_construct(
        sizeof(State*)
    );

    if (
        !states
    ) goto invalid_alloc;

    root = yar_ast_construct(
        tokens,
        err
    );

    if (
        *err
    ) goto invalid_alloc;

    fsm_fragment = fsm_fragment_construct(
        root,
        states,
        err
    );

    yar_ast_destroy(
        root
    );
    root = NULL;

    if (
        *err 
    ) goto invalid_alloc;

    fsm_fragment->final_state->is_final = 1;

    Vector *alphabet = get_alphabet(
        tokens
    );

    if (
        !alphabet
    ) goto invalid_alloc;

    assert(
        !vector_destroy(
            tokens
        )
    );

    return (FSM){
        .type           = NFA,
        .fsm_fragment   = fsm_fragment,
        .alphabet       = alphabet,
        .states         = states
    };

    invalid_alloc:

        FSM nfa = {
            .fsm_fragment   = fsm_fragment,
            .states         = states
        };

        yar_nfa_destroy(
            &nfa
        );

        if (
            root 
        ) {
            yar_ast_destroy(
                root
            );
        }

        if (
            tokens
        ) {
            assert(
                !vector_destroy(
                    tokens
                )
            );
        }

        *err |= YAR_INVALID_ALLOC;
        return (FSM){0};
}

void yar_nfa_destroy(
    FSM *nfa
){
    assert(
        nfa                 &&
        nfa->type == NFA 
    );

    if (
        nfa->alphabet
    ) {
        assert(
            !vector_destroy(
                nfa->alphabet
            )
        );
    }

    if (
        nfa->states
    ) {

        for(
            int idx = 0;
            idx < vector_get_size(
                nfa->states
            );
            idx ++
        ){
            State *curr = *((State**)vector_get(
                nfa->states,
                idx
            ));
            
            if (
                !curr 
            ) continue;

            if (
                curr->transitions
            ) {
                assert(
                    !vector_destroy(
                        curr->transitions
                    )
                );
            }
            
            free(
                curr
            );
        }
        assert(
            !vector_destroy(
                nfa->states
            )
        );
    }

    if (
        nfa->fsm_fragment
    ){
        free(
            nfa->fsm_fragment
        );
    }
    

    nfa->fsm_fragment = NULL;
    nfa->states = NULL;
}

static Vector* get_alphabet(
    Vector *tokens
){
    assert(
        tokens
    );

    Vector *alphabet = vector_construct(
        sizeof(Token)
    );

    int err = 0;

    if (
        !alphabet
    ) goto invalid_alloc;


    for(
        int token_idx = 0;
        token_idx < vector_get_size(
            tokens
        ); 
        token_idx ++
    ){
        Token curr_token = *((Token*)(vector_get(
            tokens,
            token_idx
        )));

        if (
            !is_terminal_token(
                curr_token
            ) 
        ) continue;

        int is_new_symbol = 1;

        for(
            int symbol_idx = 0;
            symbol_idx < vector_get_size(
                alphabet
            );
            symbol_idx++
        ){
            Token curr_symbol = *((Token*)(vector_get(
                alphabet,
                symbol_idx
            )));

            if (
                curr_symbol.ch == curr_token.ch
            ) {
                is_new_symbol = 0;
                break;
            }
        }

        if (
            is_new_symbol
        ) {
            err = vector_append(
                &alphabet,
                &curr_token,
                sizeof(Token)
            );
        }

        if (
            err 
        ) goto invalid_alloc;
    }

    if (
        !vector_get_size(
            alphabet
        ) 
    ) goto invalid_alloc;

    return alphabet;

    invalid_alloc:

        assert(
            !vector_destroy(
                alphabet
            )
        );

        return NULL;

}

static FsmFragment* fsm_fragment_construct(
    AstNode *root,
    Vector  *states,
    uint8_t *err
){
    assert(
        states
    );

    if (
        !root 
    ) return NULL;

    Token curr = root->op;

    FsmFragment 
    *res = NULL, 
    *left = NULL, 
    *right = NULL;

    if (
        is_terminal_token(curr)
    ) {
        res = terminal_fsm_fragment(
            curr,
            states,
            err
        );

        if (
            *err 
        ) goto invalid_alloc;

        return res;
    }

    left = fsm_fragment_construct(
        root->left,
        states,
        err
    );

    if (
        *err 
    ) goto invalid_alloc;

    right = fsm_fragment_construct(
        root->right,
        states,
        err
    );

    if (
        *err 
    ) goto invalid_alloc;

    switch (
        curr.class
    )
    {
    case CONCAT:
        res = concat_fsm_fragment(
            left, 
            right,
            states,
            err
        );
        break;
    case STAR: 
        assert(
            !right
        );
        res = star_fsm_fragment(
            left,
            states,
            err
        );
        break;
    case QMARK:
        assert(
            !right
        );
        res = qmark_fsm_fragment(
            left,
            states,
            err
        );
        break;
    case PIPE:
        res = pipe_fsm_fragment(
            left, 
            right,
            states,
            err
        );
        break;
    }   

    if (
        *err 
    ) goto invalid_alloc;

    if (
        left &&
        left != res 
    ) free(left);

    if (
        right && 
        right != res 
    ) free(right);

    return res;
    
    invalid_alloc:

        if (
            left
        ) free(left);

        if (
            right
        )free(right);

        if (
            res 
        ) free(res);
        
        return NULL;
    
}

static FsmFragment* terminal_fsm_fragment(
    Token   symbol,
    Vector  *states,
    uint8_t *err
){
    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res 
    ) goto invalid_alloc;

    res->initial_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->initial_state
    ) goto invalid_alloc;

    int vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) {
        free(
            res->initial_state
        );

        goto invalid_vec_alloc;
    }

    res->final_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->final_state
    ) goto invalid_alloc;

    vec_err = vector_append(
        &states,
        &res->final_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    Transition transition = {
        .symbol = symbol,
        .dest   = res->final_state
    };

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions
    ) goto invalid_alloc;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    return res;

    invalid_vec_alloc:

        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:
        
        if (
            res 
        ) free(res);

        *err |= YAR_INVALID_ALLOC;
        return NULL;
}

static FsmFragment* concat_fsm_fragment(
    FsmFragment *left, 
    FsmFragment *right,
    Vector      *states,
    uint8_t     *err
){
    assert(
        left &&
        right
    );
    
    int vec_err = 0;

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res 
    ) goto invalid_alloc;

    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) goto invalid_alloc;
    }

    vec_err = vector_concat(
        &left->final_state->transitions,
        right->initial_state->transitions
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    left->final_state->is_final = right->initial_state->is_final;
    
    assert(
        !vector_destroy(
            right->initial_state->transitions
        )
    );

    vec_err = vector_remove(
        &states,
        vector_find(
            states,
            &right->initial_state,
            sizeof(State*)
        )
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    free(
        right->initial_state
    );
    
    res->initial_state = left->initial_state;
    res->final_state = right->final_state;

    return res;

    invalid_vec_alloc:

        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:
        
        if (
            res 
        ) free(res);

        *err |= YAR_INVALID_ALLOC;
        return NULL;
}

static FsmFragment* pipe_fsm_fragment(
    FsmFragment *left, 
    FsmFragment *right,
    Vector      *states,
    uint8_t     *err
){
    assert(
        left &&
        right
    );

    int vec_err = 0;

    if (
        *err 
    ) return NULL;

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res
    ) goto invalid_alloc;

    Transition transition = {
        .is_empty = 1
    };

    res->initial_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->initial_state
    ) goto invalid_alloc;

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions
    ) {
        free(
            res->initial_state
        );

        goto invalid_alloc;
    }

    vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) {
        free(
            res->initial_state
        );

        goto invalid_vec_alloc;
    }

    transition.dest = left->initial_state;
    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    transition.dest = right->initial_state;
    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    res->final_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->final_state
    ) {
        free(
            res->final_state
        );
        
        goto invalid_alloc;
    }

    vec_err = vector_append(
        &states,
        &res->final_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    transition.dest = res->final_state;

    if (
        !left->final_state->transitions
    ) {
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) goto invalid_alloc;
    }

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    if (
        !right->final_state->transitions
    ){
        right->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !right->final_state->transitions
        ) goto invalid_alloc;
    }

    vec_err = vector_append(
        &right->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    return res;

    invalid_vec_alloc:

        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:
        
        if (
            res 
        ) free(res);

        *err |= YAR_INVALID_ALLOC;
        return NULL;
}

static FsmFragment* qmark_fsm_fragment(
    FsmFragment *left, 
    Vector      *states,
    uint8_t     *err
){
    assert(
        left
    );

    if (
        *err  
    ) return NULL;

    int vec_err = 0;

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res 
    ) goto invalid_alloc;

    res->initial_state = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res->initial_state
    ) goto invalid_alloc;

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions
    ) {
        free(
            res->initial_state
        );

        goto invalid_alloc;
    }

    vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) {
        free(
            res->initial_state
        );

        goto invalid_vec_alloc;
    }

    res->final_state = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res->final_state
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }

    vec_err = vector_append(
        &states,
        &res->final_state,
        sizeof(State*)
    );

    if (
        vec_err 
    ) {
        free(
            res->final_state
        );

        goto invalid_vec_alloc;
    }

    Transition transition = {
        .is_empty = 1
    };

    transition.dest = left->initial_state;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err 
    ) goto invalid_vec_alloc;

    transition.dest = res->final_state;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) goto invalid_alloc;
    }

    transition.dest = res->final_state;

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;
    
    return res;

    invalid_vec_alloc:

        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:
        
        if (
            res 
        ) free(res);

        *err |= YAR_INVALID_ALLOC;
        return NULL;

}

static FsmFragment* star_fsm_fragment(
    FsmFragment *left, 
    Vector      *states,
    uint8_t     *err
){
    assert(
        left 
    );

    if (
        *err 
    ) return NULL;

    int vec_err = 0;

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res 
    ) goto invalid_alloc;

    res->initial_state = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res->initial_state 
    ) goto invalid_alloc;

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions 
    ) {
        free(
            res->initial_state
        );
        goto invalid_alloc;
    }

    vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) {
        free(
            res->initial_state
        );
        goto invalid_vec_alloc;
    }

    res->final_state = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res->final_state 
    ) goto invalid_alloc;

    res->final_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->final_state->transitions 
    ) { 
        free(
            res->final_state
        );
        goto invalid_alloc;
    }

    vec_err = vector_append(
        &states,
        &res->final_state,
        sizeof(State*)
    );

    if (
        vec_err
    ) {
        free(
            res->final_state
        );
        goto invalid_vec_alloc;
    }

    Transition transition =  {
        .is_empty = 1
    };
    
    transition.dest = left->initial_state;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    transition.dest = res->final_state;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    if (
        !left->final_state->transitions
    ) {
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) goto invalid_alloc;
    }

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc;

    transition.dest = left->initial_state;

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    if (
        vec_err
    ) goto invalid_vec_alloc; 
    
    return res;

    invalid_vec_alloc:

        assert(
            vec_err == VEC_RESIZE_FAILED
        );

    invalid_alloc:
        
        if (
            res 
        ) free(res);

        *err |= YAR_INVALID_ALLOC;
        return NULL;

}
