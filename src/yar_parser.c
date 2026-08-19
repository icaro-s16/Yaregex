#include "yar_parser.h"


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
        .index          = 0,
        .tokens_size    = vector_get_size(tokens),
        .tokens         = tokens
    };
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