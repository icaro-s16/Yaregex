#include "yar_nfa.h"
#include <stdio.h>


FSM yar_nfa_construct(
    const char  *pattern,
    uint        *err
){
    Vector *states = vector_construct(
        sizeof(State*)
    );

    AstNode *root = yar_ast_construct(
        pattern,
        err
    );

    if (
        *err
    ) {
        yar_ast_destroy(
            root
        );

        vector_destroy(
            states
        );

        return (FSM){0};
    }

    FsmFragment *fsm_fragment = fsm_fragment_construct(
        root,
        states
    );

    yar_ast_destroy(
        root
    );

    fsm_fragment->final_state->is_final = 1;

    return (FSM){
        .fsm_fragment   = fsm_fragment,
        .states         = states
    };
}

void yar_nfa_destroy(
    FSM *fsm
){
    assert(
        fsm && 
        fsm->fsm_fragment && 
        fsm->states
    );


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

        vector_destroy(
            curr->transitions
        );
        
        free(
            curr
        );
    }

    free(
        fsm->fsm_fragment
    );

    vector_destroy(
        fsm->states
    );
}

static FsmFragment* fsm_fragment_construct(
    AstNode *root,
    Vector *states
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
        states
    );

    FsmFragment *left = fsm_fragment_construct(
        root->left,
        states
    );

    FsmFragment *right = fsm_fragment_construct(
        root->right,
        states
    );

    switch (
        curr.class
    )
    {
    case CONCAT:
        return concat_fsm_fragment(
            left, 
            right,
            states
        );
    case STAR: 
        assert(
            !right
        );
        return star_fsm_fragment(
            left,
            states
        );
    case PLUS:
        assert(
            !right
        );
        return plus_fsm_fragment(
            left,
            states
        );
    
    case QMARK:
        assert(
            !right
        );
        return qmark_fsm_fragment(
            left,
            states
        );
    case PIPE:
        return pipe_fsm_fragment(
            left, 
            right,
            states
        );
    }   
}

static FsmFragment* terminal_fsm_fragment(
    Token symbol,
    Vector *states
){
    FsmFragment *fsm = calloc(
        1,
        sizeof(FsmFragment)
    );

    fsm->initial_state = calloc(
        1,
        sizeof(State)
    );

    vector_append(
        &states,
        &fsm->initial_state
    );

    fsm->final_state = calloc(
        1,
        sizeof(State)
    );

    vector_append(
        &states,
        &fsm->final_state
    );

    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );
    transition->symbol = symbol;
    transition->dest = fsm->final_state;

    fsm->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );
    vector_append(
        &fsm->initial_state->transitions,
        transition
    );

    free(transition);
    return fsm;
}

static FsmFragment* concat_fsm_fragment(
    FsmFragment *left, 
    FsmFragment *right,
    Vector *states
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

    vector_concat(
        &left->final_state->transitions,
        right->initial_state->transitions
    );

    left->final_state->is_final = right->initial_state->is_final;
    
    vector_destroy(
        right->initial_state->transitions
    );

    vector_remove(
        &states,
        vector_find(
            states,
            &right->initial_state,
            sizeof(State*)
        )
    );

    free(
        right->initial_state
    );

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );
    
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
    Vector *states
){
    assert(
        left &&
        right
    );

    FsmFragment *res = calloc(
        1,
        sizeof(FsmFragment)
    );

    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );

    res->initial_state = calloc(
        1,
        sizeof(State)
    );

    res->initial_state->transitions = vector_construct(
        sizeof(Transition)
    );

    vector_append(
        &states,
        &res->initial_state
    );

    transition->is_empty = 1;
    transition->dest = left->initial_state;
    vector_append(
        &res->initial_state->transitions,
        transition
    );
    transition->dest = right->initial_state;
    vector_append(
        &res->initial_state->transitions,
        transition
    );

    res->final_state = calloc(
        1,
        sizeof(State)
    );

    vector_append(
        &states,
        &res->final_state
    );

    transition->dest = res->final_state;

    if (
        !left->final_state->transitions
    ) {
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );
    }
    vector_append(
        &left->final_state->transitions,
        transition
    );

    if (
        !right->final_state->transitions
    ){
        right->final_state->transitions = vector_construct(
            sizeof(Transition)
        );
    }
    vector_append(
        &right->final_state->transitions,
        transition
    );
    free(
        transition
    );
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
    Vector *states
){
    assert(
        left
    );
    
    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );

    transition->is_empty = 1;
    transition->dest = left->final_state;

    vector_append(
        &left->initial_state->transitions,
        &transition
    );

    free(
        transition
    );
    
    return left;
}

static FsmFragment* star_fsm_fragment(
    FsmFragment *left, 
    Vector *states
){
    assert(
        left 
    );

    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );
    transition->dest = left->final_state;
    transition->is_empty = 1;

    vector_append(
        &left->initial_state->transitions,
        transition
    );

    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );
    }

    transition->dest = left->initial_state;
    vector_append(
        &left->final_state->transitions,
        transition
    );
    free(
        transition
    );
    return left;
}

static FsmFragment* plus_fsm_fragment(
    FsmFragment *left, 
    Vector *states
){
    assert(
        left 
    );

    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );

    transition->is_empty = 1;
    transition->dest = left->initial_state;

    if (
        !left->final_state->transitions
    ){
        left->final_state->transitions = vector_construct(
            sizeof(Transition)
        );
    }
    
    vector_append(
        &left->final_state->transitions,
        transition
    );
    free(
        transition
    );
    
    return left;
}