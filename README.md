# Yet another regex

This is a simple regex library, created to get my hands dirty with formal language theory.

## The main flow

The regex is first compiled following these steps:

1. Scan the pattern into a vector of tokens.
2. Create the AST, following the grammar.
3. Create the Epsilon-NFA using Thompson's construction.
4. Convert the Epsilon-NFA into a DFA.

## Operators

My regex supports the main operators, which are:

1. `*`: Match zero or more symbols.
2. `+`: Match one or more symbols.
3. `{n}`: Match a sequence of n symbols.
4. `{n,}`: Match a sequence of n or more symbols.
5. `{n,m}`: Match a sequence of n to m symbols.
6. `?`: Match zero or one symbol.
7. And, of course, the regex implements an implicit concatenation of symbols.
8. `()`: Grouping to change the precedence order.

## Precedence

The precedence matches the position in the list, so 1 has the highest precedence:

1. Grouping `()`                   
2. Quantifiers `*`, `+`, `?`, `{m,n}`, `{m,}`, `{m}`   
3. Concatenation 
4. Alternation `|`                   

## Special symbols

My regex implements some special symbols, which are:

1. `[start-end]`: Match any alphanumeric symbol between the start and end. It follows the ASCII order of symbols, so the start must be lower than the end.
2. `\d`: Match any digit.
3. `\D`: Match any non-digit.
4. `\s`: Match any whitespace.
5. `\S`: Match any non-whitespace.
6. `.`: Match any symbol between 0 and 127 in the ASCII table.

**Note:** I used a naive approach for these special symbols by converting them into alternations. Because of this, symbols with large ranges like `.` can slow down the compilation of the regex.

## Regex Functions

### `int yar_match_full(YaRegex *regex, const char *text);`
Receives a text and a regex, returning true if the **entire** text matches the pattern.

### `int yar_search(YaRegex *regex, const char *text);`
Receives a text and a regex, returning true if the pattern is found anywhere within the text. The pattern does not need to match the entire string.

### `YarSlice yar_capture(YaRegex *regex, char *text);`
Receives a text and a regex. If a match is found, it returns a `YarSlice` containing the initial and final indices of the matched pattern. To find subsequent matches in the same text, call this function again passing `NULL` as the `text` pointer.

## Error Flags 

The compilation process can return a `uint8_t` containing error flags. Use bitwise `AND` (`&`) operations with these flags to identify specific errors.

*   `PARSER_INVALID_QUANTIFIER`: Raised if a quantifier is used without a preceding symbol or grouping (e.g., starting a pattern with `+`).
*   `PARSER_INVALID_GROUPING`: Raised if there is an unclosed or incomplete grouping (e.g., a missing closing parenthesis).
*   `PARSER_INVALID_ALTERNATION`: Raised if an alternation operator is used with an empty side (e.g., `a|`).
*   `SCANNER_INVALID_QUANTIFIER`: Raised if a quantifier is explicitly set to zero when not allowed.
*   `SCANNER_INVALID_RANGED_QUANTIFIER`: Raised if a ranged quantifier's start value is greater than or equal to its end value (e.g., `{5,2}`).
*   `SCANNER_INVALID_RANGED_CHAR`: Raised if a character range's starting character has a greater or equal ASCII value than its ending character (e.g., `[z-a]`).
*   `YAR_INVALID_ALLOC`: Raised if memory allocation fails during compilation.
*   `YAR_INVALID_PATTERN`: Raised if an empty pattern is provided to construct the DFA.


## References

1. To understand how [Thompson's construction](https://en.wikipedia.org/wiki/Thompson's_construction) works.
2. To understand how a scanner and a parser work, check this [page](https://github.com/DoctorWkt/acwj).
3. To understand how to convert an Epsilon-NFA directly to a DFA, check this [page](https://www.tutorialspoint.com/automata_theory/conversion_of_epsilon_nfa_to_dfa.htm).