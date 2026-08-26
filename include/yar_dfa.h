#pragma once

#include "yar_nfa.h"

FSM yar_dfa_construct(const char *pattern, uint *err);
