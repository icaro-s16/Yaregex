#include <stdio.h>
#include "yar_parser.h"

#ifdef YAR_DEBUG

/*
 * NOTE: Print the AST level by level. 
 * Each node is printed with its number 
 * of children.
 */
void parser_print_ast(
    ASTNode *root
){
    assert(
        root
    );

    Vector *queue = vector_construct(
        sizeof(ASTNode)
    );

    vector_append(
        &queue,
        root
    );

    int level_size = 1;

    while (
        vector_get_size(
            queue
        ) > 0 
    ){
        ASTNode curr = *((ASTNode*)vector_get(
            queue, 
            0
        ));

        level_size -= 1;

        vector_remove(
            &queue,
            0
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
            vector_append(
                &queue, 
                curr.left
            );
        }

        if (
            curr.right
        ) {
            vector_append(
                &queue, 
                curr.right
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
    vector_destroy(
        queue
    );
}

#endif 

/*
 * NOTE: When an AST is created, multiple syntax errors 
 * can occur. These are combined as bit flags within 
 * the err variable. To check which specific error 
 * occurred, use a bitwise AND operation (e.g., err & ERROR). 
 * If the result is non-zero, that specific ERROR occurred.
 */
ASTNode* parser_create_ast(
    Vector *tokens,
    uint *err
){
    Parser parser = parser_construct(
        tokens
    );

    ASTNode *root = alternation_expr(
        &parser
    );

    if (
        parser.state != 0
    ){
        if (
            root
        ){
            parser_destroy_ast(
                root
            );
        }
        *err = parser.state;
        root = NULL;
    }

    parser_destroy(
        &parser
    );

    return root;
}

void parser_destroy_ast(
    ASTNode *root
){
    assert(
        root
    );

    if (
        root->left
    ){
        parser_destroy_ast(
            root->left
        );
    }
    if (
        root->right
    ){
        parser_destroy_ast(
            root->right
        );
    }
    free(
        root
    );
}

static ASTNode* clone_ast(
    ASTNode* root
){
    if (
        !root
    ) return NULL;
    
    ASTNode* copy = calloc(
        1,
        sizeof(ASTNode)
    );

    copy->op = root->op;

    if (
        root->left
    ) copy->left = clone_ast(
        root->left
    );

    if (
        root->right 
    ) copy->right = clone_ast(
        root->right
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

static void parser_destroy(
    Parser *parser
){
    assert(
        parser &&
        parser->tokens
    );

    vector_destroy(
        parser->tokens
    );
}

static ASTNode* ast_node_construct(
    Token op,
    ASTNode *left,
    ASTNode *right
){
    ASTNode *node = calloc(
        1, 
        sizeof(ASTNode)
    );

    assert(
        node
    );

    node->op = op;
    node->left = left;
    node->right = right;
    
    return node;
}

static ASTNode* ast_node_construct_leaf(
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

static ASTNode* alternation_expr(
    Parser *parser
){
    assert(
        parser &&
        parser->tokens
    );
    
    ASTNode *left = NULL, *right = NULL;
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
        ) parser->state = INVALID_ALTERNATION;
        
        left = ast_node_construct(
            curr,
            left,
            right
        );

        curr = *((Token*)vector_get(
            parser->tokens,
            parser->index
        ));

    }

    return left;
}

static ASTNode* concatenation_expr(
    Parser *parser
){

    ASTNode *left = NULL, *right = NULL;
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
        parser->index += 1;

        right = quantifier_expr(
            parser
        );

        left = ast_node_construct(
            curr,
            left, 
            right
        );

        curr = *((Token*)vector_get(
            parser->tokens,
            parser->index
        ));
    }

    return left;
}

static ASTNode* translate_quantifier_exact(
    ASTNode *left,
    Token curr
){
    Token concat = {
        .class  = CONCAT,
        .attr   = NONE
    };
    
    if (
        curr.start <= 1
    )return left;
    
    ASTNode *new_left = clone_ast(
        left
    );

    for(
        int idx = 0;
        idx < curr.start - 1;
        idx ++
    ){
        ASTNode *right = clone_ast(
            new_left
        );

        left = ast_node_construct(
            concat,
            left,
            right
        );

    }
    parser_destroy_ast(
        new_left
    );

    return left;
}

static ASTNode* translate_quantifier_min(
    ASTNode *left,
    Token curr
){
    left = translate_quantifier_exact(
        left,
        curr
    );

    Token star = {
        .class  = STAR,
        .attr   = NONE
    };

    left = ast_node_construct(
        star,
        left,
        NULL
    );

    return left;
}

static ASTNode* trasnlate_ranged_quantifier(
    ASTNode *left,
    Token curr
){
    ASTNode *new_left = clone_ast(
        left
    );
    
    Token concat = {
        .class  = CONCAT,
        .attr   = NONE
    };

    Token qmark = {
        .class  = QMARK,
        .attr   = NONE
    };

    if (
        curr.start > 1
    ) {
        for(
            int idx = 0;
            idx < curr.start - 1;
            idx ++
        ){
            ASTNode *right = clone_ast(
                new_left
            );

            left = ast_node_construct(
                concat,
                left,
                right
            );

        }
    }      

    for (
        int idx = curr.start;
        idx < curr.end;
        idx++
    ){
        ASTNode *right = clone_ast(
            new_left
        );

        right = ast_node_construct(
            qmark,
            right,
            NULL
        );

        left = ast_node_construct(
            concat,
            left,
            right
        );
    }

    parser_destroy_ast(
        new_left
    );

    return left;
}

static ASTNode* quantifier_expr(
    Parser *parser
){

    ASTNode *left = NULL, *right = NULL;
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
        parser->state |= INVALID_QUANTIFIER;
        return NULL;
    }

    while (
        is_quantifier_token(
            curr
        )
    ) {
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
static ASTNode* grouping_expr(
    Parser *parser
){   
    ASTNode *left = NULL, *right = NULL;
    Token curr;

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
    
    Vector *stack = vector_construct(
        sizeof(TokenClass)
    );

    Vector *grouping_tokens = vector_construct(
        sizeof(Token)
    );

    vector_append(
        &stack,
        &curr.class
    );

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
            vector_append(
                &stack,
                &curr.class
            );
        }
        else if (
            curr.class == CLOSE_PARENTHESES
        ){
            vector_remove(
                &stack,
                vector_get_size(
                    stack
                ) - 1
            );
        }

        if (
            curr.class != CLOSE_PARENTHESES ||
            (
                curr.class == CLOSE_PARENTHESES &&
                vector_get_size(
                    stack
                ) > 0
            )
        )
            vector_append(
                &grouping_tokens,
                &curr
            );
    }

    if (
        vector_get_size(
            stack
        ) > 0
    ){
        vector_destroy(
            stack
        );
        vector_destroy(
            grouping_tokens
        );
        
        parser->state |= INVALID_GROUPING;

        /* 
         * NOTE: During the iteration, if EOT is reached, 
         * the index will exceed the vector's size. To 
         * keep it within bounds, we decrement it by one.
         */
        parser->index -= 1;                 
        return NULL;
    }

    Token eot_token = {
        .class  = EOT,
        .attr   = NONE
    };

    vector_append(
        &grouping_tokens,
        &eot_token
    );

    Parser grouping_parser = parser_construct(
        grouping_tokens
    );

    left = alternation_expr(
        &grouping_parser
    );

    vector_destroy(
        stack
    );
    parser_destroy(
        &grouping_parser
    );

    return left;
}

static ASTNode* terminal(
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
        parser->state |= INVALID_GROUPING;
    }

    if (
        !is_terminal_token(
            curr
        )
    ) return NULL;

    ASTNode *node;

    node = ast_node_construct_leaf(
        curr
    );
    parser->index ++;
    return node;
}