#pragma once 

enum {
    /* 
     * NOTE: If you write a quantifier
     * without any preceding symbol or grouping.
     */
    PARSER_INVALID_QUANTIFIER          = 1<<0,
    /* 
     * NOTE: If you write an incomplete
     * grouping.
     */
    PARSER_INVALID_GROUPING            = 1<<1,
    /* 
     * NOTE: If you write an alternation
     * with an empty side.
     */
    PARSER_INVALID_ALTERNATION         = 1<<2,
    /* 
     * NOTE: If you write a quantifier
     * with zero. 
     */
    SCANNER_INVALID_QUANTIFIER         = 1<<3,
    /* 
     * NOTE: If you write a ranged quantifier
     * in which the start point is greater or 
     * equal than the end point. 
     */
    SCANNER_INVALID_RANGED_QUANTIFIER  = 1<<4,
    /* 
     * NOTE: If you write a character range
     * in which the start char is greater or 
     * equal than the end char. 
     */
    SCANNER_INVALID_RANGED_CHAR        = 1<<5
};