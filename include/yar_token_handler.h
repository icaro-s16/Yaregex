#pragma once 

#include "yar_scanner.h"

static uint str_to_uint(Scanner* scanner);

Token char_tokens_handler(Scanner* scanner);

Token char_range_handler(Scanner *scanner);

Token quantifier_handler(Scanner *scanner);

Token backslash_handler(Scanner *scanner);