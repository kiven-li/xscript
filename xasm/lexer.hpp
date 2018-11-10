#ifndef     __XSCRIPT_LEXER_HPP__
#define     __XSCRIPT_LEXER_HPP__

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "utility.hpp"
#include "instruction_set.hpp"

using std::string;

namespace xscript {
namespace xasm {

//line limit & identifier limit
#define     MAX_SOURCE_LINE_SIZE    8192//maximum source line length
#define     MAX_LEXEME_SIZE         256//maximum lexeme size

enum TOKEN_TYPE
{
    TOKEN_TYPE_INT = 0, //An integer literal
    TOKEN_TYPE_FLOAT, //1 An floating-point literal
    TOKEN_TYPE_STRING, //2 An string literal
    TOKEN_TYPE_QUOTE, //3 An double-quote
    TOKEN_TYPE_IDENT, //4 An identifier
    TOKEN_TYPE_COLON, //5 A colon
    TOKEN_TYPE_OPEN_BRACKET, //6 An openening bracket
    TOKEN_TYPE_CLOSE_BRACKET, //7 An closing bracket
    TOKEN_TYPE_COMMA, //8 A comma
    TOKEN_TYPE_OPEN_BRACE, //9 An openening curly brace
    TOKEN_TYPE_CLOSE_BRACE, //10 An closing curly brace
    TOKEN_TYPE_NEWLINE, //11 A newline

    TOKEN_TYPE_INSTR, //12 An instruction

    TOKEN_TYPE_SETSTACKSIZE, //13 The SetStateSize directive
    TOKEN_TYPE_SETPRIORITY, //14 The SetPriority directive
    TOKEN_TYPE_VAR, //15 The Var/ Var [] directives
    TOKEN_TYPE_FUNC, //16 The Func directives
    TOKEN_TYPE_PARAM, //17 The Param directives
    TOKEN_TYPE_REG_RETVAL, //18 The _RetVal directives

    TOKEN_TYPE_INVALID, //19 Error code for invalid tokens
    END_OF_TOKEN_STREAM, //20 The end of the stream has been reached
};

enum LEXER_STATE
{
    LEX_STATE_START = 0,

    LEX_STATE_INT,
    LEX_STATE_FLOAT,

    LEX_STATE_IDENT,

    LEX_STATE_INSTR,
    LEX_STATE_DELIM, 

    LEX_STATE_STRING,
    LEX_STATE_STRING_ESCAPE,
    LEX_STATE_STRING_CLOSE_QUOTE,
};

//lexical
typedef int token;
typedef std::vector<string> string_vector;

class lexer
{
public:
    explicit lexer(instruction_set& inn);
    ~lexer();

    //file
    void load_source_file(const string& source_file);
    int get_souce_code_line_count() { return source_codes.size(); }

    //lexical analysis
    token get_current_token() { return current_token; }
    char* get_current_lexeme() { return current_lexeme; }
    void reset_lexer();
    bool skip_to_next_line();
    char read_look_ahead_char();

    char get_next_char();
    token read_next_token();

    //error
    void exit_on_error(const char* emsg);
    void exit_on_code_error(const char* emsg);
    void exit_on_char_expected_error(char c);
private:
    //instruction set
    instruction_set& iset;

    //source code
    string_vector source_codes;

    //lexer state info
    int current_source_line;
    int index_start;
    int index_end;

    token current_token;
    char current_lexeme[MAX_LEXEME_SIZE];

    int current_lex_state;
};

}//xasm
}//xscript

#endif      //__XSCRIPT_LEXER_HPP__
