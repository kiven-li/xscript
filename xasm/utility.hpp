#ifndef     __XSCRIPT_XSTRING_HPP__
#define     __XSCRIPT_XSTRING_HPP__

#include <string.h>
#include <ctype.h>

//char
bool is_char_whitespace(char c);
bool is_char_numeric(char c);
bool is_char_identifier(char c);
bool is_char_delimiter(char c);

//string
char* strupr(char* str);
void trim_whitespace(char* str);
void strip_comments(char* str);

bool is_string_whitespace(char* str);
bool is_string_indentifier(char* str);
bool is_string_integer(char* str);
bool is_string_float(char* str);

#endif      //__XSCRIPT_XSTRING_HPP__
