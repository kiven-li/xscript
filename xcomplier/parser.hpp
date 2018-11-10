#ifndef     __XSCRIPT_XCOMPLIER_PARSER_HPP__
#define     __XSCRIPT_XCOMPLIER_PARSER_HPP__

#include "lexer.hpp"
#include "i_code.hpp"
#include <stack>

#define MAX_FUNC_DECLARE_PARAM_COUNT        32// The maximum number of parameters hat can appear in a function

namespace xscript {
namespace xcomplier {

class xcomplier;

struct loop_instance
{
    int start_index;
    int end_index;
};
typedef std::stack<loop_instance> xstack;

class parser
{
public:
    parser(xcomplier& x, lexer& lx, x_icode& xi);
    ~parser();

    void parse_source_code();

private:
    void read_token(token t);

    bool is_operand_relational(int ot);
    bool is_operand_logical(int ot);

    void parse_statement();
    void parse_block();

    void parse_var();
    void parse_host();
    void parse_function();

    void parse_expression();
    void parse_sub_expression();
    void parse_term();
    void parse_factor();

    void parse_if();
    void parse_while();
    void parse_for();
    void parse_break();
    void parse_continue();
    void parse_return();

    void parse_assign();
    void parse_function_call();
private:

    void ParseAssign ();
    void parse_functionCall ();
private:
    int current_scope;
    xstack loop_stack;
    xcomplier& xcom;
    lexer& lex;
    x_icode& xicode;
};

}//namespace xcomplier
}//namespace xscript

#endif      //__XSCRIPT_XCOMPLIER_PARSER_HPP__
