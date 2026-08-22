#include <stdio.h>
#include "yar_parser.h"

#ifdef YAR_DEBUG

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

        /* NOTE: In the interation, if reach EOT, the index will pass the  
         * size of the vector, so to continue inside of it, we substract it 
         * by one.
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