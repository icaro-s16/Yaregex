#include "yar_nfa.h"


FSM yar_nfa_construct(
    const char  *pattern,
    uint8_t     *err
){

    Vector *tokens = yar_scan(
        pattern,
        err
    );

    if (
        *err 
    ) return (FSM){0};
    

    Vector *states = vector_construct(
        sizeof(State*)
    );

    if (
        !states
    ) {
        assert(
            !vector_destroy(
                tokens
            )
        );
        *err |= YAR_INVALID_ALLOC;
        return (FSM){0};
    }

    AstNode *root = yar_ast_construct(
        tokens,
        err
    );

    if (
        *err
    ) {
        if (
            root
        ){
            yar_ast_destroy(
                root
            );
        }
        assert(
            !vector_destroy(
                states
            ) &&
            !vector_destroy(
                tokens
            )
        );

        return (FSM){0};
    }

    FsmFragment *fsm_fragment = fsm_fragment_construct(
        root,
        states,
        err
    );

    yar_ast_destroy(
        root
    );

    if (
        *err 
    ) {
        FSM nfa = {
            .type           = NFA,
            .fsm_fragment   = fsm_fragment,
            .states         = states
        };
        
        yar_nfa_destroy(
            &nfa    
        );

        assert(
            !vector_destroy(
                tokens
            )
        );

        return (FSM){0};

    }

    if (
        *err 
    ) {
        FSM nfa = {
            .fsm_fragment   = fsm_fragment,
            .states         = states
        };
        yar_nfa_destroy(
            &nfa
        );

        assert(
            !vector_destroy(
                tokens
            )
        );

        return (FSM){0};
    }

    fsm_fragment->final_state->is_final = 1;

    Vector *alphabet = get_alphabet(
        tokens
    );

    if (
        !alphabet
    ) {
        FSM nfa = {
            .fsm_fragment   = fsm_fragment,
            .states         = states
        };
        yar_nfa_destroy(
            &nfa
        );

        assert(
            !vector_destroy(
                tokens
            )
        );

        *err |= YAR_INVALID_ALLOC;
        return (FSM){0};
    }

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
}

void yar_nfa_destroy(
    FSM *fsm
){
    assert(
        fsm                 &&
        fsm->type == NFA 
    );

    if (
        fsm->states
    ) {

        for(
            int idx = 0;
            idx < vector_get_size(
                fsm->states
            );
            idx ++
        ){
            State *curr = *((State**)vector_get(
                fsm->states,
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
                fsm->states
            )
        );
    }

    if (
        fsm->fsm_fragment
    ){
        free(
            fsm->fsm_fragment
        );
    }
    

    fsm->fsm_fragment = NULL;
    fsm->states = NULL;
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
    ) return NULL;


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
        ) {
            assert(
                !vector_destroy(
                    alphabet
                )
            );
            return NULL;
        }
    }

    if (
        !vector_get_size(
            alphabet
        ) 
    ) {
        assert(
            !vector_destroy(
                alphabet
            )
        );
        return NULL;
    }

    return alphabet;
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

    if (
        is_terminal_token(curr)
    ) return terminal_fsm_fragment(
        curr,
        states,
        err
    );

    FsmFragment *left = fsm_fragment_construct(
        root->left,
        states,
        err
    );

    if (
        *err 
    ) return NULL;

    FsmFragment *right = fsm_fragment_construct(
        root->right,
        states,
        err
    );

    if (
        *err 
    ) return NULL;

    switch (
        curr.class
    )
    {
    case CONCAT:
        return concat_fsm_fragment(
            left, 
            right,
            states,
            err
        );
    case STAR: 
        assert(
            !right
        );
        return star_fsm_fragment(
            left,
            states,
            err
        );
    case PLUS:
        assert(
            !right
        );
        return plus_fsm_fragment(
            left,
            states,
            err
        );
    
    case QMARK:
        assert(
            !right
        );
        return qmark_fsm_fragment(
            left,
            states,
            err
        );
    case PIPE:
        return pipe_fsm_fragment(
            left, 
            right,
            states,
            err
        );
    }   
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
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    res->initial_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->initial_state
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    int vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->initial_state
        );
        free(
            res
        );
        return NULL;
    }

    res->final_state = calloc(
        1,
        sizeof(State)
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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }

    Transition transition = {
        .symbol = symbol,
        .dest   = res->final_state
    };

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    return res;
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
    
    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );
    }

    int vec_err = vector_concat(
        &left->final_state->transitions,
        right->initial_state->transitions
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |=  YAR_INVALID_ALLOC;
        return NULL;
    }

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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    free(
        right->initial_state
    );

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res 
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }
    
    res->initial_state = left->initial_state;
    res->final_state = right->final_state;
    free(
        left
    );
    free(
        right
    );

    return res;
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
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    Transition transition = {
        .is_empty = 1
    };

    res->initial_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->initial_state
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->initial_state
        );
        free(
            res
        );
        return NULL;
    }

    vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->initial_state
        );
        free(
            res
        );
        return NULL;
    }

    transition.dest = left->initial_state;
    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }

    transition.dest = right->initial_state;
    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }

    res->final_state = calloc(
        1,
        sizeof(State)
    );

    if (
        !res->final_state
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->final_state
        );
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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    transition.dest = res->final_state;

    if (
        !left->final_state->transitions
    ) {
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) {
            *err |= YAR_INVALID_ALLOC;
            free(
                res
            );
            return NULL;
        }
    }
    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ){
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    if (
        !right->final_state->transitions
    ){
        right->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !right->final_state->transitions
        ) {
            *err |= YAR_INVALID_ALLOC;
            free(
                res
            );
            return NULL;
        }
    }
    vec_err = vector_append(
        &right->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    free(
        right
    );
    free(
        left
    );
    return res;
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

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    int vec_err = 0;

    if (
        !res 
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    res->initial_state = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res->initial_state
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->initial_state
        );
        free(
            res 
        );
        return NULL;
    }

    vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        assert(
            !vector_destroy(
                res->initial_state->transitions
            )
        );   
        free(
            res->initial_state
        );
        free(
            res 
        );
        return NULL;
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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err 
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->final_state
        );
        free(
            res 
        );
        return NULL;
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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err 
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }

    transition.dest = res->final_state;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }

    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) {
            *err |= YAR_INVALID_ALLOC;
            free(
                res 
            );
            return NULL;
        }
    }

    transition.dest = res->final_state;

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res 
        );
        return NULL;
    }
    
    return res;
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
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    res->initial_state = calloc(
        1,
        sizeof(FsmFragment)
    );

    if (
        !res->initial_state 
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    } 

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->initial_state->transitions 
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->initial_state
        );
        free(
            res
        );
        return NULL;
    }

    vec_err = vector_append(
        &states,
        &res->initial_state,
        sizeof(State*)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        assert(
            !vector_destroy(
                res->initial_state->transitions
            )
        );
        free(
            res->initial_state
        );
        free(
            res
        );
        return NULL;
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

    res->final_state->transitions = vector_construct(
        sizeof(Transition)
    );

    if (
        !res->final_state->transitions 
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res->final_state
        );
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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        assert(
            !vector_destroy(
                res->final_state->transitions
            )
        );
        free(
            res->final_state
        );
        free(
            res
        );
        return NULL;
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

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    transition.dest = res->final_state;

    vec_err = vector_append(
        &res->initial_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }

    if (
        !left->final_state->transitions
    ) {
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) {
            *err = YAR_INVALID_ALLOC;
            free(
                res
            );
            return NULL;
        }
    }

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return left;
    }

    transition.dest = left->initial_state;

    vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        free(
            res
        );
        return NULL;
    }
    
    return res;
}

static FsmFragment* plus_fsm_fragment(
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

    Transition transition = {
        .is_empty = 1
    };

    transition.dest = left->initial_state;

    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );

        if (
            !left->final_state->transitions
        ) {
            *err |= YAR_INVALID_ALLOC;
            return NULL;
        }
    }
    
    int vec_err = vector_append(
        &left->final_state->transitions,
        &transition,
        sizeof(Transition)
    );

    assert(
        !vec_err ||
        vec_err == VEC_RESIZE_FAILED
    );

    if (
        vec_err
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    return left;
}