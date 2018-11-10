#ifndef     __XSCRIPT_XCOMPLIER_LEXER_HPP__
#define     __XSCRIPT_XCOMPLIER_LEXER_HPP__

#include <string.h>
#include <string>
#include <vector>

using std::string;
typedef std::vector<std::string> string_vector;

#define MAX_LEXEME_SIZE                 1024//Maximum individual lexeme size
#define MAX_OP_STATE_COUNT              32//Maximum number of operator states
#define MAX_DELIM_COUNT                 24//Maximum number of delimiters

enum LEXER_STATE
{
    LEX_STATE_UNKNOWN = 0,// Unknown lexeme type
    LEX_STATE_START,//1 Start state

    LEX_STATE_INT,//2 Integer
    LEX_STATE_FLOAT,//3 Float

    LEX_STATE_IDENT,//4 Identifier

    LEX_STATE_OP,//5 Operator
    LEX_STATE_DELIM,//6 Delimiter    

    LEX_STATE_STRING,//7 String value
    LEX_STATE_STRING_ESCAPE,//8 Escape sequence
    LEX_STATE_STRING_CLOSE_QUOTE,//9 String closing quote
};

enum TOKEN_TYPE
{
    TOKEN_TYPE_END_OF_STREAM = 0,// End of the token stream
    TOKEN_TYPE_INVALID,//1 Invalid token

    TOKEN_TYPE_INT,//2 Integer
    TOKEN_TYPE_FLOAT,//3 Float

    TOKEN_TYPE_IDENT,//4 Identifier

    TOKEN_TYPE_RSRVD_VAR,//5 var/var []
    TOKEN_TYPE_RSRVD_TRUE,//6 true
    TOKEN_TYPE_RSRVD_FALSE,//7 false
    TOKEN_TYPE_RSRVD_IF,//8 if
    TOKEN_TYPE_RSRVD_ELSE,//9 else
    TOKEN_TYPE_RSRVD_BREAK,//10 break
    TOKEN_TYPE_RSRVD_CONTINUE,//11 continue
    TOKEN_TYPE_RSRVD_FOR,//12 for
    TOKEN_TYPE_RSRVD_WHILE,//13 while
    TOKEN_TYPE_RSRVD_FUNC,//14 func
    TOKEN_TYPE_RSRVD_RETURN,//15 return
    TOKEN_TYPE_RSRVD_HOST,//16 host

    TOKEN_TYPE_OP,//17 Operator

    TOKEN_TYPE_DELIM_COMMA,//18 ,
    TOKEN_TYPE_DELIM_OPEN_PAREN,//19 (
    TOKEN_TYPE_DELIM_CLOSE_PAREN,//20 )
    TOKEN_TYPE_DELIM_OPEN_BRACE,//21 [
    TOKEN_TYPE_DELIM_CLOSE_BRACE,//22 ]
    TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE,//23 {
    TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE,//24 }
    TOKEN_TYPE_DELIM_SEMICOLON,//25 ;

    TOKEN_TYPE_STRING,//26 String
};

enum OPERAND_TYPE
{
    OP_TYPE_ADD = 0,// +
    OP_TYPE_SUB = 1,// -
    OP_TYPE_MUL = 2,// *
    OP_TYPE_DIV = 3,// /
    OP_TYPE_MOD = 4,// %
    OP_TYPE_EXP = 5,// ^
    OP_TYPE_CONCAT = 35,// $

    OP_TYPE_INC = 15,// ++
    OP_TYPE_DEC = 17,// --

    OP_TYPE_ASSIGN_ADD = 14,// +=
    OP_TYPE_ASSIGN_SUB = 16,// -=
    OP_TYPE_ASSIGN_MUL = 18,// *=
    OP_TYPE_ASSIGN_DIV = 19,// /=
    OP_TYPE_ASSIGN_MOD = 20,// %=
    OP_TYPE_ASSIGN_EXP = 21,// ^=
    OP_TYPE_ASSIGN_CONCAT = 36,// $=

    //---- Bitwise
    OP_TYPE_BITWISE_AND = 6,// &
    OP_TYPE_BITWISE_OR = 7,// |
    OP_TYPE_BITWISE_XOR = 8,// #
    OP_TYPE_BITWISE_NOT = 9,// ~
    OP_TYPE_BITWISE_SHIFT_LEFT = 30,// <<
    OP_TYPE_BITWISE_SHIFT_RIGHT = 32,// >>

    OP_TYPE_ASSIGN_AND = 22,// &=
    OP_TYPE_ASSIGN_OR = 24,// |=
    OP_TYPE_ASSIGN_XOR = 26,// #=
    OP_TYPE_ASSIGN_SHIFT_LEFT = 33,// <<=
    OP_TYPE_ASSIGN_SHIFT_RIGHT = 34,// >>=

    //---- Logical
    OP_TYPE_LOGICAL_AND = 23,// &&
    OP_TYPE_LOGICAL_OR = 25,// ||
    OP_TYPE_LOGICAL_NOT = 10,// !

    //---- Relational
    OP_TYPE_EQUAL = 28,// ==
    OP_TYPE_NOT_EQUAL = 27,// !=
    OP_TYPE_LESS = 12,// <
    OP_TYPE_GREATER = 13,// >
    OP_TYPE_LESS_EQUAL = 29,// <=
    OP_TYPE_GREATER_EQUAL = 31,// >=

    //---- Assignment
    OP_TYPE_ASSIGN = 11,// =
};

namespace xscript {
namespace xcomplier {

typedef int token;

struct lexer_state
{
    int line_index;
    string line;

    token current_token;

    char lexeme[MAX_LEXEME_SIZE];
    int lexeme_start;
    int lexeme_end;

    int current_operand;
};

struct operand_state
{
    char cc;
    int sub_state_index;
    int sub_state_count;
    int index;
};

class lexer
{
public:
    lexer() {};
    ~lexer() {};

    int get_source_line_count() { return source_code.size(); }
    void load_source_file(const string& file_name);
    void preprocess_source_file();

    void reset();
    void copy_lexer_state(lexer_state& d, lexer_state& s);

    int get_operand_state_index(char c, int cindex, int ssindex, int sscount);
    bool is_char_operand_char(char c, int cindex);
    operand_state get_operand_state(int cindex, int sindex);

    bool is_char_delim(char c);
    bool is_char_whitespace(char c);
    bool is_char_numeric(char c);
    bool is_char_identifier(char c);

    char get_next_char();
    char get_look_ahead_char();
    token get_next_token();

    void rewind_token_stream() { copy_lexer_state(current, prev); }
    token get_current_token() { return current.current_token; }
    char* get_current_lexeme() { return current.lexeme; }
    void copy_current_lexeme(char* buf) { strcpy(buf, current.lexeme); }
    int get_current_operand() { return current.current_operand; }

    string get_current_source_line() { return current.line; }
    int get_current_source_line_index() { return current.line_index; }
    int get_lexeme_start_index() { return current.lexeme_start; }
private:
    string_vector source_code;

    lexer_state current;                    // The current lexer state
    lexer_state prev;
};

}//namespace xcomplier
}//namespace xscript

#endif      //__XSCRIPT_XCOMPLIER_LEXER_HPP__
