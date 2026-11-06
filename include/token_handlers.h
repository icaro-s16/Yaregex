#pragma once 

#include "scanner.h"

Token char_tokens_handler(Scanner *scanner);

Vector* ranged_char_handler(Scanner *scanner);

Token symbols_group_handler(Scanner *scanner);

Token quantifier_handler(Scanner *scanner);

Token backslash_handler(Scanner *scanner);

static uint str_to_uint(Scanner* scanner);

static void append_number_symbols(Scanner *scanner, Vector *symbols);

static int append_symbols(Vector *tokens, Token symbol);

static Token invalid_syntax(Scanner *scanner, Vector *symbols, char curr);