#pragma once 

#include "yar_scanner.h"

static uint str_to_uint(Scanner* scanner);

static void append_number_symbols(const Scanner *scanner, Vector **symbols);

static void handler_append_symbol(Vector **tokens, Token symbol);

static Token handle_invalid_operation_syntax(Scanner *scanner, Vector *symbols, char curr);

Token char_tokens_handler(Scanner* scanner);

Token ranged_char_handler(Scanner *scanner);

Token quantifier_handler(Scanner *scanner);

Token backslash_handler(Scanner *scanner);