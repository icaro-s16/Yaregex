#pragma once

#include "yar_scanner.h"

enum{
    INVALID_QUANTIFIER  = 1<<0,
    INVALID_GROUPING    = 1<<1,
    INVALID_ALTERNATION = 1<<2
};

typedef struct ASTNode ASTNode;
struct ASTNode{
    Token op;
    ASTNode *left, *right;
};

typedef struct{
    uint    state;
    size_t  index;
    Vector  *tokens;
}Parser;

#ifdef YAR_DEBUG

void parser_print_ast(ASTNode *root);

#endif

ASTNode* parser_create_ast(Vector *tokens, uint *err);

void parser_destroy_ast(ASTNode *root);

static Parser parser_construct(Vector *tokens);

static void parser_destroy(Parser *parser); 

static ASTNode* ast_node_construct(Token op, ASTNode *left, ASTNode *right);

static ASTNode* ast_node_construct_leaf(Token op);

static ASTNode* alternation_expr(Parser *parser);

static ASTNode* concatenation_expr(Parser *parser);

static ASTNode* quantifier_expr(Parser *parser);

static ASTNode* grouping_expr(Parser *parser);

static ASTNode* terminal(Parser *parser);