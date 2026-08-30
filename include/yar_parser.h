#pragma once

#include "yar_scanner.h"

typedef struct AstNode AstNode;
struct AstNode{
    Token op;
    AstNode *left, *right;
};

typedef struct{
    uint8_t state;
    size_t  index;
    Vector  *tokens;
}Parser;

#ifdef YAR_DEBUG

void yar_print_ast(AstNode *root);

#endif

AstNode* yar_ast_construct(Vector *tokens, uint8_t *err);

void yar_ast_destroy(AstNode *root);

static AstNode* clone_ast(AstNode* root);

static Parser parser_construct(Vector *tokens);

static AstNode* ast_node_construct(Token op, AstNode *left, AstNode *right);

static AstNode* ast_node_construct_leaf(Token op);

static AstNode* alternation_expr(Parser *parser);

static AstNode* concatenation_expr(Parser *parser);

static AstNode* translate_quantifier_exact(AstNode *left, Token curr);

static AstNode* translate_quantifier_min(AstNode *left, Token curr);

static AstNode* trasnlate_ranged_quantifier(AstNode *left, Token curr);

static AstNode* quantifier_expr(Parser *parser);

static AstNode* grouping_expr(Parser *parser);

static AstNode* terminal(Parser *parser);