#include <stdio.h>
#include "yar_parser.h"

#ifdef YAR_DEBUG

/*
 * NOTE: Print the AST level by level. 
 * Each node is printed with its number 
 * of children.
 */
void yar_print_ast(
    AstNode *root
){
    assert(
        root
    );

    Vector *queue = vector_construct(
        sizeof(AstNode)
    );

    assert(
        queue
    );

    assert(
        !vector_append(
            &queue,
            root,
            sizeof(AstNode)
        )
    );

    int level_size = 1;

    while (
        vector_get_size(
            queue
        ) > 0 
    ){
        AstNode curr = *((AstNode*)vector_get(
            queue, 
            0
        ));

        level_size -= 1;

        assert(
            !vector_remove(
                &queue,
                0
            )
        );

        int counter = 0;
        if (
            curr.left
        ) counter += 1;
        if (
            curr.right
        ) counter += 1;

        printf(
            "%s{%d}%c", 
            yar_token_to_string(
                curr.op
            ), 
            counter, 
            (!level_size) ? '\n' : ' '
        );

        if (
            curr.left
        ) {
            assert(
                !vector_append(
                    &queue, 
                    curr.left,
                    sizeof(AstNode)
                )
            );
        }

        if (
            curr.right
        ) {
            assert(
                !vector_append(
                    &queue, 
                    curr.right,
                    sizeof(AstNode)
                )
            );
        }

        if (
            !level_size
        ) {
            level_size = vector_get_size(
                queue
            );
        }
        
    }
    assert(
        !vector_destroy(
            queue
        )
    );
}

#endif 

/*
 * NOTE: When an AST is created, multiple syntax errors 
 * can occur. These are combined as bit flags within 
 * the err variable. To check which specific error 
 * occurred, use a bitwise AND operation (e.g., err & ERROR). 
 * If the result is non-zero, that specific ERROR occurred.
 * 
 */
AstNode* yar_ast_construct(
    Vector      *tokens,
    uint8_t     *err
){

    assert(
        tokens && 
        err
    );

    if (
        *err 
    ){
        return NULL;
    }

    Parser parser = parser_construct(
        tokens
    );

    AstNode *root = alternation_expr(
        &parser
    );

    if (
        parser.state
    ){
        if (
            root
        ){
            yar_ast_destroy(
                root
            );
        }
        *err |= parser.state;
        root = NULL;
    }

    return root;
}

void yar_ast_destroy(
    AstNode *root
){
    assert(
        root
    );

    if (
        root->left
    ){
        yar_ast_destroy(
            root->left
        );
    }
    if (
        root->right
    ){
        yar_ast_destroy(
            root->right
        );
    }
    free(
        root
    );
    root = NULL;
}

static AstNode* clone_ast(
    AstNode*    root,
    uint8_t     *err
){
    if (
        !root
    ) return NULL;
    
    AstNode* copy = calloc(
        1,
        sizeof(AstNode)
    );

    if (
        !copy
    ) {
        *err |= YAR_INVALID_ALLOC;
        return NULL;
    }

    copy->op = root->op;

    if (
        root->left
    ) copy->left = clone_ast(
        root->left,
        err 
    );

    if (
        root->right 
    ) copy->right = clone_ast(
        root->right,
        err
    );

    return copy;    
}

static Parser parser_construct(
    Vector *tokens
){
    assert(
        tokens && 
        vector_get_data_type_size(
            tokens
        ) == sizeof(Token)
    );

    return (Parser){
        .state          = 0,
        .index          = 0,
        .tokens         = tokens
    };
}

static AstNode* ast_node_construct(
    Token op,
    AstNode *left,
    AstNode *right
){
    AstNode *node = calloc(
        1, 
        sizeof(AstNode)
    );

    if (
        !node 
    ) return NULL;

    node->op = op;
    node->left = left;
    node->right = right;
    
    return node;
}

static AstNode* ast_node_construct_leaf(
    Token op
){
    assert(
        is_terminal_token(
            op
        )
    );

    return ast_node_construct(
        op,
        NULL,
        NULL
    );
}

static AstNode* alternation_expr(
    Parser *parser
){
    assert(
        parser &&
        parser->tokens
    );
    
    AstNode *left = NULL, *right = NULL, *new_left = NULL;
    Token curr;

    left = concatenation_expr(
        parser
    );

    curr = *((Token*)vector_get(
        parser->tokens,
        parser->index
    ));

    if (
        curr.class == EOT 
    ) return left;


    while (
        curr.class != EOT
    ){  
        parser->index += 1;

        right = concatenation_expr(
            parser
        );

        if (
            !left ||
            !right 
        ) parser->state |= PARSER_INVALID_ALTERNATION;
        
        new_left = ast_node_construct(
            curr,
            left, 
            right
        );

        if (
            !new_left
        ) goto invalid_alloc;

        left = new_left;

        curr = *((Token*)vector_get(
            parser->tokens,
            parser->index
        ));

    }

    return left;

    invalid_alloc:

        if (
            new_left
        ) {
            yar_ast_destroy(
                new_left
            );
        }

        if (
            right
        ) {
            yar_ast_destroy(
                right
            );
        }

        if (
            left
        ) {
            yar_ast_destroy(
                left
            );
        }

        parser->state |= YAR_INVALID_ALLOC;
        return NULL;
}

static AstNode* concatenation_expr(
    Parser *parser
){

    AstNode *left = NULL, *right = NULL, *new_left = NULL;
    Token curr;

    left = quantifier_expr(
        parser
    );

    curr = *((Token*)vector_get(
        parser->tokens,
        parser->index
    ));

    if (
        curr.class == EOT 
    ) return left;

    while (
        curr.class == CONCAT 
    ){
        right = new_left = NULL;
        parser->index += 1;

        right = quantifier_expr(
            parser
        );

        new_left = ast_node_construct(
            curr,
            left, 
            right
        );

        if (
            !new_left
        ) goto invalid_alloc;

        left = new_left;

        curr = *((Token*)vector_get(
            parser->tokens,
            parser->index
        ));

    }

    return left;

    invalid_alloc:

        if (
            new_left
        ) {
            yar_ast_destroy(
                new_left
            );
        }

        if (
            right
        ) {
            yar_ast_destroy(
                right
            );
        }

        if (
            left
        ) {
            yar_ast_destroy(
                left
            );
        }

        parser->state |= YAR_INVALID_ALLOC;
        return NULL;
}

static AstNode* translate_quantifier_exact(
    AstNode *left,
    Token curr
){
    Token concat = {
        .class  = CONCAT
    };
    AstNode *copy_node = NULL, *right = NULL, *new_left = NULL;
    uint8_t err = 0;

    if (
        curr.start <= 1
    )return left;
    
    copy_node = clone_ast(
        left,
        &err 
    );

    if (
        err 
    ) goto invalid_alloc;

    for(
        int idx = 0;
        idx < curr.start - 1;
        idx ++
    ){
        right = new_left = NULL;

        right = clone_ast(
            copy_node,
            &err 
        );

        if (
            err 
        ) goto invalid_alloc;

        new_left = ast_node_construct(
            concat,
            left,
            right
        );

        if (
            !new_left 
        ) goto invalid_alloc;

        left = new_left;

    }

    yar_ast_destroy(
        copy_node
    );

    return left;

    invalid_alloc:

        if (
            copy_node
        ) {
            yar_ast_destroy(
                copy_node
            );
        }

        if (
            new_left
        ) {
            yar_ast_destroy(
                new_left
            );
        }

        if (
            right
        ) {
            yar_ast_destroy(
                right
            );
        }

        if (
            left
        ) {
            yar_ast_destroy(
                left
            );
        }

        return NULL;
}

static AstNode* translate_quantifier_min(
    AstNode *left,
    Token curr
){
    
    uint8_t err = 0;
    AstNode *new_right = NULL, *new_left = NULL;
    AstNode *copy_node = clone_ast(
        left,
        &err 
    );

    if (
        err 
    ) goto invalid_alloc;
    
    curr.start - 1;
    new_left = translate_quantifier_exact(
        left,
        curr
    );

    if (
        !new_left
    ) goto invalid_alloc;

    left = new_left;
    new_left = NULL;
    
    Token concat = {
        .class = CONCAT
    };

    Token star = {
        .class = STAR
    };

    new_right = ast_node_construct(
        star,
        copy_node,
        NULL
    );

    if (
        !new_right
    ) goto invalid_alloc;

    new_left = ast_node_construct(
        concat,
        left,
        new_right
    );

    if (
        !new_left
    ) goto invalid_alloc;

    left = new_left;

    return left;

    invalid_alloc:

        if (
            left 
        ) {
            yar_ast_destroy(
                left
            );
        }
        if (
            new_left
        ) {
            yar_ast_destroy(
                new_left
            );
        }
        if (
            new_right
        ){
            yar_ast_destroy(
                new_right
            );
        }
        if (
            copy_node
        ) {
            yar_ast_destroy(
                copy_node
            );
        }

        return NULL;
}

static AstNode* trasnlate_ranged_quantifier(
    AstNode *left,
    Token curr
){
    uint8_t err = 0;

    AstNode *right = NULL, *new_left = NULL, *new_right = NULL;

    AstNode *copy_node = clone_ast(
        left,
        &err 
    );
    
    if (
        err 
    ) goto invalid_alloc;

    Token concat = {
        .class  = CONCAT
    };

    Token qmark = {
        .class  = QMARK
    };

    if (
        curr.start > 1
    ) {
        for(
            int idx = 0;
            idx < curr.start - 1;
            idx ++
        ){
            new_left = right = NULL;

            right = clone_ast(
                copy_node,
                &err
            );

            if (
                err 
            ) goto invalid_alloc;
            
            AstNode *new_left = ast_node_construct(
                concat,
                left,
                right
            );

            if (
                !new_left
            ) goto invalid_alloc;

            left = new_left;

        }
    }      

    for (
        int idx = curr.start;
        idx < curr.end;
        idx++
    ){
        new_right = new_left = right = NULL;
        
        right = clone_ast(
            copy_node,
            &err
        );
        
        if (
            err 
        ) goto invalid_alloc;

        AstNode* new_right = ast_node_construct(
            qmark,
            right,
            NULL
        );

        if (
            !new_right 
        ) goto invalid_alloc;

        right = new_right;

        new_left = ast_node_construct(
            concat,
            left,
            right
        );

        if (
            !new_left
        ) goto invalid_alloc;

        left = new_left;
    }

    yar_ast_destroy(
        copy_node
    );

    return left;

    invalid_alloc:

        if ( 
            copy_node
        ) {
            yar_ast_destroy(
                copy_node
            );
        }
        if (
            new_left
        ){
            yar_ast_destroy(
                new_left
            );
        }
        if (
            right
        ) {
            yar_ast_destroy(
                right
            );
        }
        if (
            left
        ) {
            yar_ast_destroy(
                left
            );
        }
        return NULL;
}

static AstNode* quantifier_expr(
    Parser *parser
){

    AstNode *left = NULL, *right = NULL;
    Token curr;

    left = grouping_expr(
        parser
    );

    curr = *((Token*)vector_get(
        parser->tokens,
        parser->index
    ));

    if (
        curr.class == EOT 
    ) return left;

    if (
        !left
    ){
        parser->state |= PARSER_INVALID_QUANTIFIER;
        return NULL;
    }

    while (
        is_quantifier_token(
            curr
        )
    ) {
        assert(
            !right
        );

        parser->index += 1;

        /*
         * NOTE: This approach allows us to build a default 
         * FSM using only the {*, +, ?, |, .} operators. It 
         * is easier to construct and avoids the need to 
         * deep-copy a cyclic graph.
         */
        switch (curr.class)
        {
        case QUANTIFIER_EXACT:
            left = translate_quantifier_exact(
                left,
                curr
            );
            break;
        case QUANTIFIER_MIN:
            left = translate_quantifier_min(
                left,
                curr
            );
            break;
        case RANGED_QUANTIFIER:
            left = trasnlate_ranged_quantifier(
                left,
                curr
            );
            break;
        default:
            left = ast_node_construct(
                curr,
                left,
                NULL
            );
            break;
        }

        if (
            !left
        ) {
            parser->state |= YAR_INVALID_ALLOC;
            return NULL;
        }

        curr = *((Token*)vector_get(
            parser->tokens,
            parser->index
        ));
    }

    return left;
}

/* 
 * TODO: This function is too expensive. 
 * There might be another way to implement 
 * it without using so many allocations.
 */
static AstNode* grouping_expr(
    Parser *parser
){   
    AstNode *left = NULL, *right = NULL;
    
    Vector *grouping_tokens = NULL, *stack = NULL;

    Token curr;

    int err = 0;

    left = terminal(
        parser
    );

    curr = *((Token*)vector_get(
        parser->tokens,
        parser->index
    ));

    if (
        curr.class == EOT 
    ) return left;


    if (
        curr.class != OPEN_PARENTHESES
    ) return left;
    
    stack = vector_construct(
        sizeof(TokenClass)
    );

    if (
        !stack
    ) goto invalid_alloc;

    grouping_tokens = vector_construct(
        sizeof(Token)
    );

    if (
        !grouping_tokens
    ) goto invalid_alloc;

    err = vector_append(
        &stack,
        &curr.class,
        sizeof(TokenClass)
    );

    if (
        err 
    ) goto invalid_vec_alloc;

    parser->index += 1;

    for(
        ;
        curr.class != EOT &&
        vector_get_size(
            stack
        ) > 0 ;
        parser->index ++
    ){
        curr = *((Token*)vector_get(
            parser->tokens,
            parser->index
        ));

        if (
            curr.class == OPEN_PARENTHESES
        ){
            err = vector_append(
                &stack,
                &curr.class,
                sizeof(TokenClass)
            );

            if (
                !err
            ) {
                err = vector_append(
                    &grouping_tokens,
                    &curr,
                    sizeof(Token)
                );
            }
        }
        else if (
            curr.class == CLOSE_PARENTHESES
        ){
            err = vector_remove(
                &stack,
                vector_get_size(
                    stack
                ) - 1
            );

            if (
                vector_get_size(
                    stack
                ) > 0 &&
                !err 
            ) {
                err = vector_append(
                    &grouping_tokens,
                    &curr,
                    sizeof(Token)
                );
            }
        }
        else {
            err = vector_append(
                &grouping_tokens,
                &curr,
                sizeof(Token)
            );
        }

        if (
            err 
        ) goto invalid_vec_alloc;

    }

    if (
        vector_get_size(
            stack
        ) > 0 ||
        vector_get_size(
            grouping_tokens
        ) == 0
    ){
        assert(
            !vector_destroy(
                stack
            ) &&
            !vector_destroy(
                grouping_tokens
            )
        );
        
        parser->state |= PARSER_INVALID_GROUPING;

        /* 
         * NOTE: During the iteration, if EOT is reached, 
         * the index will exceed the vector's size. To 
         * keep it within bounds, we decrement it by one.
         */
        parser->index -= 1;                 
        return NULL;
    }

    Token eot_token = {
        .class  = EOT
    };

    err = vector_append(
        &grouping_tokens,
        &eot_token,
        sizeof(Token)
    );

    if (
        err 
    ) goto invalid_vec_alloc;

    Parser grouping_parser = parser_construct(
        grouping_tokens
    );

    if (
        grouping_parser.state
    ) goto invalid_alloc;

    if (
        !(
            left = alternation_expr(
                &grouping_parser
            )
        )
    ) parser->state |= PARSER_INVALID_GROUPING;

    parser->state |= grouping_parser.state;

    assert(
        !vector_destroy(
            stack
        ) &&
        !vector_destroy(
            grouping_tokens
        )
    );

    return left;

    invalid_vec_alloc:

        assert(
            err == VEC_RESIZE_FAILED
        );
    
    invalid_alloc:

        if (
            stack
        ) {
            assert(
                !vector_destroy(
                    stack
                ) 
            );
        }

        if (
            grouping_tokens
        ) {
            assert(
                !vector_destroy(
                    grouping_tokens
                )
            );
        }
        
        parser->state |= YAR_INVALID_ALLOC;
        return NULL;
}

static AstNode* terminal(
    Parser *parser
){
    assert(
        parser &&
        parser->tokens
    );
    Token curr = *((Token*)vector_get(
        parser->tokens,
        parser->index
    ));

    /* 
     * NOTE: If a CLOSE_PARENTHESES reaches this function, 
     * it likely means the grouping function did not find 
     * the corresponding OPEN_PARENTHESES.
     */
    if (
        curr.class == CLOSE_PARENTHESES 
    ){
        parser->state |= PARSER_INVALID_GROUPING;
    }

    if (
        !is_terminal_token(
            curr
        )
    ) return NULL;

    AstNode *node = NULL;

    node = ast_node_construct_leaf(
        curr
    );

    if (
        !node 
    ) {
        parser->state |= YAR_INVALID_ALLOC;
        return NULL;
    };

    parser->index ++;
    return node;
}