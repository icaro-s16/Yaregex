#pragma once

#include "yar_scanner.h"


typedef struct ASTNode ASTNode;
struct ASTNode{
    Token op;
    ASTNode *left, *right;
};

typedef struct{
    size_t  index;
    size_t  tokens_size;
    Vector  *tokens;
}Parser;

static Parser parser_construct(Vector *tokens);

static ASTNode* ast_node_construct(Token op, ASTNode *left, ASTNode *right);

static ASTNode* ast_node_construct_leaf(Token op);
