#include "yar_nfa.h"


FSM* yar_create_automaton(
    ASTNode *root
){
    if (
        !root 
    ) return NULL;

    Token curr = root->op;

    if (
        is_terminal_token(curr)
    ) return terminal_fsm(curr);

    FSM *left = yar_create_automaton(
        root->left
    );

    FSM *right = yar_create_automaton(
        root->right
    );

    switch (
        curr.class
    )
    {
    case CONCAT:
        return concat_fsm(
            left, 
            right
        );
    case STAR: 
        return star_fsm(
            left,
            right
        );
    case PLUS:
        return plus_fsm(
            left,
            right
        );
    
    case QMARK:
        return qmark_fsm(
            left,
            right
        );
    case PIPE:
        return pipe_fsm(
            left, 
            right
        );
    }   
}

static FSM* terminal_fsm(
    Token symbol
){
    FSM *fsm = calloc(
        1,
        sizeof(FSM)
    );

    fsm->initial_state = calloc(
        1,
        sizeof(State)
    );
    fsm->final_state = calloc(
        1,
        sizeof(State)
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
        &transition
    );

    free(transition);
    return fsm;
}

static FSM* concat_fsm(
    FSM *left, 
    FSM *right
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
    free(
        right->initial_state
    );

    FSM *res = calloc(
        1,
        sizeof(FSM)
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

static FSM* pipe_fsm(
    FSM *left, 
    FSM *right
){
    assert(
        left &&
        right
    );

    FSM *res = calloc(
        1,
        sizeof(FSM)
    );

    Vector *transitions = vector_construct(
        sizeof(Transition)
    );
    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );

    res->initial_state = calloc(
        1,
        sizeof(State)
    );
    transition->is_empty = 1;
    transition->dest = left->initial_state;
    vector_append(
        &transitions,
        transition
    );
    transition->dest = right->initial_state;
    vector_append(
        &transitions,
        transition
    );

    res->initial_state->transitions = transitions;

    res->final_state = calloc(
        1,
        sizeof(State)
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

static FSM* qmark_fsm(
    FSM *left, 
    FSM *right
){
    assert(
        left &&
        !right
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

static FSM* star_fsm(
    FSM *left, 
    FSM *right
){
    assert(
        left &&
        !right
    );

    Transition *transition = calloc(
        1,
        sizeof(Transition)
    );
    transition->dest = left->final_state;
    transition->is_empty = 1;

    vector_append(
        &left->initial_state->transitions,
        &transition
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
        &transition
    );
    free(
        transition
    );
    return left;
}

static FSM* plus_fsm(
    FSM *left, 
    FSM *right
){
    assert(
        left &&
        !right
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