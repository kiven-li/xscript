#include "i_code.hpp"
#include "parser.hpp"
#include "xcomplier.hpp"

namespace xscript {
namespace xcomplier {

parser::parser(xcomplier& x, lexer& lx, x_icode& xi)
: loop_stack(),
  xcom(x),
  lex(lx),
  xicode(xi)
{
}

parser::~parser()
{ 
}

void parser::parse_source_code()
{
    lex.reset();
    current_scope = SCOPE_GLOBAL;

    while(true)
    {
        parse_statement();

        if(lex.get_next_token() == TOKEN_TYPE_END_OF_STREAM)
        {
            break;
        }
        else
        {
            lex.rewind_token_stream();
        }
    }
}

void parser::read_token(token t)
{
    if(lex.get_next_token() != t)
    {
        string error_msg;
        switch(t)
        {
        case TOKEN_TYPE_INT:
            error_msg = "Integer";
            break;
        case TOKEN_TYPE_FLOAT:
            error_msg = "Float";
            break;
        case TOKEN_TYPE_IDENT:
            error_msg = "Identifier";
            break;
        case TOKEN_TYPE_RSRVD_VAR:
            error_msg = "var";
            break;
        case TOKEN_TYPE_RSRVD_TRUE:
            error_msg = "true";
            break;
        case TOKEN_TYPE_RSRVD_FALSE:
            error_msg = "false";
            break;
        case TOKEN_TYPE_RSRVD_IF:
            error_msg = "if";
            break;
        case TOKEN_TYPE_RSRVD_ELSE:
            error_msg = "else";
            break;
        case TOKEN_TYPE_RSRVD_BREAK:
            error_msg = "break";
            break;
        case TOKEN_TYPE_RSRVD_CONTINUE:
            error_msg = "continue";
            break;
        case TOKEN_TYPE_RSRVD_FOR:
            error_msg = "for";
            break;
        case TOKEN_TYPE_RSRVD_WHILE:
            error_msg = "while";
            break;
        case TOKEN_TYPE_RSRVD_FUNC:
            error_msg = "func";
            break;
        case TOKEN_TYPE_RSRVD_RETURN:
            error_msg = "return";
            break;
        case TOKEN_TYPE_RSRVD_HOST:
            error_msg = "host";
            break;
        case TOKEN_TYPE_OP:
            error_msg = "Operator";
            break;
        case TOKEN_TYPE_DELIM_COMMA:
            error_msg = ",";
            break;
        case TOKEN_TYPE_DELIM_OPEN_PAREN:
            error_msg = "(";
            break;
        case TOKEN_TYPE_DELIM_CLOSE_PAREN:
            error_msg = ")";
            break;
        case TOKEN_TYPE_DELIM_OPEN_BRACE:
            error_msg = "[";
            break;
        case TOKEN_TYPE_DELIM_CLOSE_BRACE:
            error_msg = "]";
            break;
        case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
            error_msg = "{";
            break;
        case TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE:
            error_msg = "}";
            break;
        case TOKEN_TYPE_DELIM_SEMICOLON:
            error_msg = ";";
            break;
        case TOKEN_TYPE_STRING:
            error_msg = "String";
            break;
        }

        error_msg += "expected";
        xcom.exit_on_code_error(error_msg.c_str());
    }
}

bool parser::is_operand_relational(int ot)
{
    if( ot != OP_TYPE_EQUAL &&
        ot != OP_TYPE_NOT_EQUAL &&
        ot != OP_TYPE_LESS &&
        ot != OP_TYPE_GREATER &&
        ot != OP_TYPE_LESS_EQUAL &&
        ot != OP_TYPE_GREATER_EQUAL)
    {
        return false;
    }

    return true;
}

bool parser::is_operand_logical(int ot)
{
    if( ot != OP_TYPE_LOGICAL_AND &&
        ot != OP_TYPE_LOGICAL_OR &&
        ot != OP_TYPE_LOGICAL_NOT)
    {
        return false;
    }

    return true;
}

void parser::parse_statement()
{
    //if the next token is a semicolon, the statement is empty so return
    if(lex.get_look_ahead_char() == ';')
    {
        read_token(TOKEN_TYPE_DELIM_SEMICOLON);
        return;
    }

    token it = lex.get_next_token();
    switch(it)
    {
    case TOKEN_TYPE_END_OF_STREAM:
        xcom.exit_on_code_error("Unexpected end of file");
        break;
    case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
        parse_block();
        break;
    case TOKEN_TYPE_RSRVD_VAR:
        parse_var();
        break;
    case TOKEN_TYPE_RSRVD_HOST:
        parse_host();
        break;
    case TOKEN_TYPE_RSRVD_FUNC:
        parse_function();
        break;
    case TOKEN_TYPE_RSRVD_IF:
        parse_if();
        break;
    case TOKEN_TYPE_RSRVD_WHILE:
        parse_while();
        break;
    case TOKEN_TYPE_RSRVD_FOR:
        parse_for();
        break;
    case TOKEN_TYPE_RSRVD_BREAK:
        parse_break();
        break;
    case TOKEN_TYPE_RSRVD_CONTINUE:
        parse_continue();
        break;
    case TOKEN_TYPE_RSRVD_RETURN:
        parse_return();
        break;
    case TOKEN_TYPE_IDENT:
    {
        if(xcom.get_symbol_by_ident(lex.get_current_lexeme(), current_scope))
        {
            parse_assign();
        }
        else if(xcom.get_function_by_name(lex.get_current_lexeme()))
        {
            xicode.add_icode_source_line(current_scope, lex.get_current_source_line());
            parse_function_call();

            read_token(TOKEN_TYPE_DELIM_SEMICOLON);
        }
        else
        {
            xcom.exit_on_code_error("Invalid identifier");
        }
        break;
    }
    default:
        xcom.exit_on_code_error("Unexpected input");
        break;
    }//switch(it)   
}

//{ <Statement-List> }
void parser::parse_block()
{
    if(current_scope == SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("Code blocks illegal in global scope");
    }

    while(lex.get_look_ahead_char() != '}')
    {
        parse_statement();
    }

    read_token(TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE);
}

//var <Identifier>;
//var <Identifier>[<Integer>];
void parser::parse_var()
{
    read_token(TOKEN_TYPE_IDENT);

    string identifier = lex.get_current_lexeme();

    int size = 1;
    if(lex.get_look_ahead_char() == '[')
    {
        read_token(TOKEN_TYPE_DELIM_OPEN_BRACE);
        read_token(TOKEN_TYPE_INT);

        size = atoi(lex.get_current_lexeme());

        read_token(TOKEN_TYPE_DELIM_CLOSE_BRACE);
    }

    if(xcom.add_symbol(identifier.c_str(), size, current_scope, SYMBOL_TYPE_VAR) == -1)
    {
        xcom.exit_on_code_error("Identifier redefinition");
    }

    read_token(TOKEN_TYPE_DELIM_SEMICOLON);    
}

//host <Identifier>();
void parser::parse_host()
{
    read_token(TOKEN_TYPE_IDENT);

    if(xcom.add_function(lex.get_current_lexeme(), true) == -1)
    {
        xcom.exit_on_code_error("Function redefinition");
    }

    read_token(TOKEN_TYPE_DELIM_OPEN_PAREN);
    read_token(TOKEN_TYPE_DELIM_CLOSE_PAREN);
    read_token(TOKEN_TYPE_DELIM_SEMICOLON);   
}

//function <Identifier> (<Parameter-List>) <Statement>
void parser::parse_function()
{
    if(current_scope != SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("Nested functions illegal");
    }

    //function name
    read_token(TOKEN_TYPE_IDENT);

    int function_index = xcom.add_function(lex.get_current_lexeme(), false);
    if(function_index == -1)
    {
        xcom.exit_on_code_error("Function redefinition");
    }
    current_scope = function_index;

    read_token(TOKEN_TYPE_DELIM_OPEN_PAREN);
    if(lex.get_look_ahead_char() != ')')
    {
        if(xcom.xheader.is_main_function_present && xcom.xheader.main_function_index == function_index)
        {
            xcom.exit_on_code_error("_Main() connot accept parameters");
        }

        int param_count = 0;
        char param_list[MAX_FUNC_DECLARE_PARAM_COUNT][MAX_IDENT_SIZE];
        while(true)
        {
            read_token(TOKEN_TYPE_IDENT);
            lex.copy_current_lexeme(param_list[param_count]);
            ++param_count;

            if(lex.get_look_ahead_char() == ')')
            {
                break;
            }

            read_token(TOKEN_TYPE_DELIM_COMMA);
        }
        xcom.set_function_param_count(current_scope, param_count);

        while(param_count > 0)
        {
            --param_count;
            xcom.add_symbol(param_list[param_count], 1, current_scope, SYMBOL_TYPE_PARAM);
        }
    }

    read_token(TOKEN_TYPE_DELIM_CLOSE_PAREN);
    read_token(TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE);

    parse_block();

    current_scope = SCOPE_GLOBAL;
}

void parser::parse_expression()
{
    int instruction_index;
    int operand_type;

    parse_sub_expression();

    while(true)
    {
        if(lex.get_next_token() != TOKEN_TYPE_OP ||
            (!is_operand_relational(lex.get_current_operand()) &&
             !is_operand_logical(lex.get_current_operand())))
        {
            lex.rewind_token_stream();
            break;
        }

        operand_type = lex.get_current_operand();

        parse_sub_expression();

        //pop the first operand into _T1
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);

        //pop the second operand into _T0
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);

        //perform the binary operation associated with the specified operator
        //determine the operator type
        if(is_operand_relational(operand_type))
        {
            int true_jump_target_index = xicode.get_next_jump_target_index();
            int exit_jump_target_index = xicode.get_next_jump_target_index();

            switch(operand_type)
            {
            case OP_TYPE_EQUAL:
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JE);
                break;
            case OP_TYPE_NOT_EQUAL:
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JNE);
                break;
            case OP_TYPE_GREATER:
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JG);
                break;
            case OP_TYPE_LESS:
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JL);
                break;
            case OP_TYPE_GREATER_EQUAL:
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JGE);
                break;
            case OP_TYPE_LESS_EQUAL:
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JLE);
                break;
            }

            //add the jump instruction's operands (_T0 and _T1)
            xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
            xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);
            xicode.add_jump_target_icode_op(current_scope, instruction_index, true_jump_target_index);

            //generate the outcome for false hood
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
            xicode.add_int_icode_op(current_scope, instruction_index, 0);

            //generate a jump past the true outcome
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JMP);
            xicode.add_jump_target_icode_op(current_scope, instruction_index, exit_jump_target_index);

            //set the jump target for the true outcome
            xicode.add_icode_jump_target(current_scope, true_jump_target_index);

            //generate the outcome for truth
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
            xicode.add_int_icode_op(current_scope, instruction_index, 1);

            //set the jump target for exiting the operand evaluation
            xicode.add_icode_jump_target(current_scope, exit_jump_target_index);
        }
        else
        {
            //it must be a logical operator
            switch(operand_type)
            {
            case OP_TYPE_LOGICAL_AND:
            {
                int false_jump_target_index = xicode.get_next_jump_target_index();
                int exit_jump_target_index = xicode.get_next_jump_target_index();

                //JE _t0, 0, true
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JE);
                xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
                xicode.add_int_icode_op(current_scope, instruction_index, 0);
                xicode.add_jump_target_icode_op(current_scope, instruction_index, false_jump_target_index);

                //JE _t1, 0, true
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JE);
                xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);
                xicode.add_int_icode_op(current_scope, instruction_index, 0);
                xicode.add_jump_target_icode_op(current_scope, instruction_index, false_jump_target_index);

                //push 1
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                xicode.add_int_icode_op(current_scope, instruction_index, 1);

                //jmp exit
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JMP);
                xicode.add_jump_target_icode_op(current_scope, instruction_index, exit_jump_target_index);

                //L0: false
                xicode.add_icode_jump_target(current_scope, false_jump_target_index);

                //push 0
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                xicode.add_int_icode_op(current_scope, instruction_index, 0);

                //L1:exit
                xicode.add_icode_jump_target(current_scope, exit_jump_target_index);

                break;
            }
            case OP_TYPE_LOGICAL_OR:
            {
                int true_jump_target_index = xicode.get_next_jump_target_index();
                int exit_jump_target_index = xicode.get_next_jump_target_index();

                //JNE _T0, 0, true
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JNE);
                xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
                xicode.add_int_icode_op(current_scope, instruction_index, 0);
                xicode.add_jump_target_icode_op(current_scope, instruction_index, true_jump_target_index);

                //JNE _T1, 0, true
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JNE);
                xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);
                xicode.add_int_icode_op(current_scope, instruction_index, 0);
                xicode.add_jump_target_icode_op(current_scope, instruction_index, true_jump_target_index);

                //push 0
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                xicode.add_int_icode_op(current_scope, instruction_index, 0);

                //jump exit
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JMP);
                xicode.add_jump_target_icode_op(current_scope, instruction_index, exit_jump_target_index);

                //L0:true
                xicode.add_icode_jump_target(current_scope, true_jump_target_index);

                //push 1
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                xicode.add_int_icode_op(current_scope, instruction_index, 1);

                //L1:exit
                xicode.add_icode_jump_target(current_scope, exit_jump_target_index);
                break;
            }
            }//switch(operand_type)
        }
    }
}

void parser::parse_sub_expression() 
{
    int instruction_index;
    int operand_type;

    parse_term();
    while(true)
    {
        if(lex.get_next_token() != TOKEN_TYPE_OP ||
            (lex.get_current_operand() != OP_TYPE_ADD &&
             lex.get_current_operand() != OP_TYPE_SUB &&
             lex.get_current_operand() != OP_TYPE_CONCAT))
        {
            lex.rewind_token_stream();
            break;
        }

        operand_type = lex.get_current_operand();

        parse_term();

        //pop the first operand into _T1
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);

        //pop the second operand int _T0
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);

        int instruction;
        switch(operand_type)
        {
        case OP_TYPE_ADD:
            instruction = INSTR_ADD;
            break;
        case OP_TYPE_SUB:
            instruction = INSTR_SUB;
            break;
        case OP_TYPE_CONCAT:
            instruction = INSTR_CONCAT;
            break;
        }

        instruction_index = xicode.add_icode_instruction(current_scope, instruction);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);

        //push the result
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
    }
}

void parser::parse_term() 
{
    int instruction_index;
    int operand_type;

    parse_factor();

    while(true)
    {
        if(lex.get_next_token() != TOKEN_TYPE_OP ||
            (lex.get_current_operand() != OP_TYPE_MUL &&
             lex.get_current_operand() != OP_TYPE_DIV &&
             lex.get_current_operand() != OP_TYPE_MOD &&
             lex.get_current_operand() != OP_TYPE_EXP &&
             lex.get_current_operand() != OP_TYPE_BITWISE_AND &&
             lex.get_current_operand() != OP_TYPE_BITWISE_OR &&
             lex.get_current_operand() != OP_TYPE_BITWISE_XOR &&
             lex.get_current_operand() != OP_TYPE_BITWISE_SHIFT_LEFT &&
             lex.get_current_operand() != OP_TYPE_BITWISE_SHIFT_RIGHT))
        {
            lex.rewind_token_stream();
            break;
        }

        operand_type = lex.get_current_operand();

        parse_factor();

        //pop the first operand into _T1
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);

        //pop the second operand into _T0
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);

        int instruction;
        switch(operand_type)
        {
        case OP_TYPE_MUL:
            instruction = INSTR_MUL;
            break;
        case OP_TYPE_DIV:
            instruction = INSTR_DIV;
            break;
        case OP_TYPE_MOD:
            instruction = INSTR_MOD;
            break;
        case OP_TYPE_EXP:
            instruction = INSTR_EXP;
            break;
        case OP_TYPE_BITWISE_AND:
            instruction = INSTR_AND;
            break;
        case OP_TYPE_BITWISE_OR:
            instruction = INSTR_OR;
            break;
        case OP_TYPE_BITWISE_XOR:
            instruction = INSTR_XOR;
            break;
        case OP_TYPE_BITWISE_SHIFT_LEFT:
            instruction = INSTR_SHL;
            break;
        case OP_TYPE_BITWISE_SHIFT_RIGHT:
            instruction = INSTR_SHR;
            break;
        }

        instruction_index = xicode.add_icode_instruction(current_scope, instruction);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_1_symbol_index);

        //push the result 
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
    }
}

void parser::parse_factor() 
{
    int instruction_index;
    int operand_type;
    bool is_unary_operand_pending = false;

    if(lex.get_next_token() == TOKEN_TYPE_OP &&
        (lex.get_current_operand() == OP_TYPE_ADD ||
         lex.get_current_operand() == OP_TYPE_SUB ||
         lex.get_current_operand() == OP_TYPE_BITWISE_NOT ||
         lex.get_current_operand() == OP_TYPE_LOGICAL_NOT))
    {
        is_unary_operand_pending = true;
        operand_type = lex.get_current_operand();
    }
    else
    {
        lex.rewind_token_stream();
    }

    //Determine which type of factor we're dealing with based on the next token
    switch(lex.get_next_token())
    {
    case TOKEN_TYPE_RSRVD_TRUE:
    case TOKEN_TYPE_RSRVD_FALSE:
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
        xicode.add_int_icode_op(current_scope, instruction_index, lex.get_current_token () == TOKEN_TYPE_RSRVD_TRUE ? 1 : 0);
        break;
    case TOKEN_TYPE_INT:
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
        xicode.add_int_icode_op(current_scope, instruction_index, atoi(lex.get_current_lexeme()));
        break;
    case TOKEN_TYPE_FLOAT:
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
        xicode.add_float_icode_op(current_scope, instruction_index, atof(lex.get_current_lexeme()));
        break;
    case TOKEN_TYPE_STRING:
    {
        int string_index = xcom.add_string(lex.get_current_lexeme());
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
        xicode.add_string_icode_op(current_scope, instruction_index, string_index);
        break;
    }
    case TOKEN_TYPE_IDENT:
    {
        symbol* s = xcom.get_symbol_by_ident(lex.get_current_lexeme(), current_scope);
        if(s)
        {
            if(lex.get_look_ahead_char() == '[')
            {
                if(s->size == 1)
                {
                    xcom.exit_on_code_error("Invalid array");
                }

                read_token(TOKEN_TYPE_DELIM_OPEN_BRACE);

                if(lex.get_look_ahead_char() == ']')
                {
                    xcom.exit_on_code_error("Invalid expression");
                }

                parse_expression();
                read_token(TOKEN_TYPE_DELIM_CLOSE_BRACE);

                //pop the resulting value into _T0 and use it as the index variable
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
                xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);

                //push the original identifier onto the stack as an array, indexed with _T0
                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                xicode.add_array_index_var_icode_op(current_scope, instruction_index, s->index, xcom.temp_var_0_symbol_index);
            }
            else
            {
                if(s->size == 1)
                {
                    instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                    xicode.add_var_icode_op(current_scope, instruction_index, s->index);
                }
                else
                {
                    xcom.exit_on_code_error("Array must be indexed");
                }
            }
        }
        else
        {
            if(xcom.get_function_by_name(lex.get_current_lexeme()))
            {
                parse_function_call();

                instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
                xicode.add_reg_icode_op(current_scope, instruction_index, REG_CODE_RETVAL);
            }
        }
        break;
    }
    case TOKEN_TYPE_DELIM_OPEN_PAREN:
        parse_expression();
        read_token (TOKEN_TYPE_DELIM_CLOSE_PAREN);
        break;
    default:
        xcom.exit_on_code_error("Invalid input");
    }

    if(is_unary_operand_pending)
    {
        instruction_index = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);

        if(operand_type == OP_TYPE_LOGICAL_NOT)
        {
            int true_jump_target_index = xicode.get_next_jump_target_index();
            int exit_jump_target_index = xicode.get_next_jump_target_index();

            //JE _T0, 0, true
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JE);
            xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
            xicode.add_int_icode_op(current_scope, instruction_index, 0);
            xicode.add_jump_target_icode_op(current_scope, instruction_index, true_jump_target_index);

            //push 0
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
            xicode.add_int_icode_op(current_scope, instruction_index, 0);

            //jmp L1
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_JMP);
            xicode.add_jump_target_icode_op(current_scope, instruction_index, exit_jump_target_index);

            //L0:true
            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
            xicode.add_int_icode_op(current_scope, instruction_index, 1);

            //L1:exit
            xicode.add_icode_jump_target(current_scope, exit_jump_target_index);
        }
        else
        {
            int instruction;
            switch(operand_type)
            {
            case OP_TYPE_SUB:
                instruction = INSTR_NEG;
                break;
            case OP_TYPE_BITWISE_NOT:
                instruction = INSTR_NOT;
                break;
            }

            instruction_index = xicode.add_icode_instruction(current_scope, instruction);
            xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);

            instruction_index = xicode.add_icode_instruction(current_scope, INSTR_PUSH);
            xicode.add_var_icode_op(current_scope, instruction_index, xcom.temp_var_0_symbol_index);
        }
    }
}

//if(<Expression>) <Statement>
//if(<Expression>) <Statement> else <Statement>
void parser::parse_if() 
{
    int ii;

    if(current_scope == SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("if illegal in global scope");
    }

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    int false_jump_target_index = xicode.get_next_jump_target_index();

    read_token(TOKEN_TYPE_DELIM_OPEN_PAREN);
    parse_expression();
    read_token(TOKEN_TYPE_DELIM_CLOSE_PAREN);

    //pop the result into _T0 and compare it to zero
    ii = xicode.add_icode_instruction(current_scope, INSTR_POP);
    xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);

    //if the result is zero, jump to the false target
    ii = xicode.add_icode_instruction(current_scope, INSTR_JE);
    xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);
    xicode.add_int_icode_op(current_scope, ii, 0);
    xicode.add_jump_target_icode_op(current_scope, ii, false_jump_target_index);

    //parse the true block
    parse_statement();

    //look for an else caluse
    if(lex.get_next_token() == TOKEN_TYPE_RSRVD_ELSE)
    {
        //if it's found, append the true block with an unconditional jump past the false block
        int skip_false_jump_target_index = xicode.get_next_jump_target_index();
        ii = xicode.add_icode_instruction(current_scope, INSTR_JMP);
        xicode.add_jump_target_icode_op(current_scope, ii, skip_false_jump_target_index);

        //place the false target just before the false block
        xicode.add_icode_jump_target(current_scope, false_jump_target_index);

        //parse the false block
        parse_statement();

        //set a jump target beyond the false block
        xicode.add_icode_jump_target(current_scope, skip_false_jump_target_index);
    }
    else
    {
        lex.rewind_token_stream();

        //place the false target after the true block
        xicode.add_icode_jump_target(current_scope, false_jump_target_index);
    }
}

//while(<Expression>) <Statement>
void parser::parse_while() 
{
    int ii;

    if(current_scope == SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("Statement illegal in global scope");
    }

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    int start_target_index = xicode.get_next_jump_target_index();
    int end_target_index = xicode.get_next_jump_target_index();

    xicode.add_icode_jump_target(current_scope, start_target_index);

    read_token(TOKEN_TYPE_DELIM_OPEN_PAREN);
    parse_expression();
    read_token(TOKEN_TYPE_DELIM_CLOSE_PAREN);

    //pop the result into _T0 and jump out of the loop if it's nonzero
    ii = xicode.add_icode_instruction(current_scope, INSTR_POP);
    xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);

    ii = xicode.add_icode_instruction(current_scope, INSTR_JE);
    xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);
    xicode.add_int_icode_op(current_scope, ii, 0);
    xicode.add_jump_target_icode_op(current_scope, ii, end_target_index);

    loop_instance loop;
    loop.start_index = start_target_index;
    loop.end_index = end_target_index;
    loop_stack.push(loop);

    //parse the loop body
    parse_statement();

    loop_stack.pop();

    //unconditionally jump back to the start of the loop
    ii = xicode.add_icode_instruction(current_scope, INSTR_JMP);
    xicode.add_jump_target_icode_op(current_scope, ii, start_target_index);

    //set a jump target for the end of the loop
    xicode.add_icode_jump_target(current_scope, end_target_index);   
}

//for(<Initializer>; <Condition>; <Perpetuator>) <Statement>
void parser::parse_for() 
{
    if(current_scope == SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("Statement illegal in global scope");
    }

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    //TODO A for loop parser implementation could go here  
}

void parser::parse_break() 
{
    if(loop_stack.empty())
    {
        xcom.exit_on_code_error("break illegal in outside loops");
    }

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    read_token(TOKEN_TYPE_DELIM_SEMICOLON);

    int target_index = loop_stack.top().end_index;

    int ii = xicode.add_icode_instruction(current_scope, INSTR_JMP);
    xicode.add_jump_target_icode_op(current_scope, ii, target_index);
}

void parser::parse_continue() 
{
    if(loop_stack.empty())
    {
        xcom.exit_on_code_error("break illegal in outside loops");
    }

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    read_token(TOKEN_TYPE_DELIM_SEMICOLON);

    int target_index = loop_stack.top().start_index;

    int ii = xicode.add_icode_instruction(current_scope, INSTR_JMP);
    xicode.add_jump_target_icode_op(current_scope, ii, target_index);
}

//return;
//return <expr>;
void parser::parse_return() 
{
    int ii;

    if(current_scope == SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("return illegal in global scope");
    }

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    //if a semiconon doesn't appear to follow, parse the expression and place it in _RetVal
    if(lex.get_look_ahead_char() != ';')
    {
        //parse the expression to calculate the return value and leave the result on the stack.
        parse_expression();

        //determine which function we're returning from
        if(xcom.xheader.is_main_function_present && xcom.xheader.main_function_index == current_scope)
        {
            ii = xicode.add_icode_instruction(current_scope, INSTR_POP);
            xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);
        }
        else
        {
            ii = xicode.add_icode_instruction(current_scope, INSTR_POP);
            xicode.add_reg_icode_op(current_scope, ii, REG_CODE_RETVAL);
        }
    }
    else
    {
        //clear _T0 in case we're exiting _Main()
        if(xcom.xheader.is_main_function_present && xcom.xheader.main_function_index == current_scope)
        {
            ii = xicode.add_icode_instruction(current_scope, INSTR_MOV);
            xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);
            xicode.add_int_icode_op(current_scope, ii, 0);
        }
    }

    if(xcom.xheader.is_main_function_present && xcom.xheader.main_function_index == current_scope)
    {
        ii = xicode.add_icode_instruction(current_scope, INSTR_EXIT);
        xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);
    }
    else
    {
        xicode.add_icode_instruction(current_scope, INSTR_RET);
    }

    read_token(TOKEN_TYPE_DELIM_SEMICOLON);
}

//<Ident> <Assign-Op> <Expr>;
void parser::parse_assign()
{
    if(current_scope == SCOPE_GLOBAL)
    {
        xcom.exit_on_code_error("Assignment illegal in global scope");
    }

    int ii;
    int assign_operand;

    xicode.add_icode_source_line(current_scope, lex.get_current_source_line());

    symbol* s = xcom.get_symbol_by_ident(lex.get_current_lexeme(), current_scope);
    bool is_array = false;
    if(lex.get_look_ahead_char() == '[')
    {
        if(s->size == 1)
        {
            xcom.exit_on_code_error("Invalid array");
        }

        read_token(TOKEN_TYPE_DELIM_OPEN_BRACE);

        if(lex.get_look_ahead_char() == ']')
        {
            xcom.exit_on_code_error("Invalid expression");
        }

        parse_expression();
        read_token(TOKEN_TYPE_DELIM_CLOSE_BRACE);

        is_array = true;
    }
    else
    {
        if(s->size > 1)
        {
            xcom.exit_on_code_error("Arrays must be indexed");
        }
    }

    if(lex.get_next_token() != TOKEN_TYPE_OP &&
      (lex.get_current_operand() != OP_TYPE_ASSIGN &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_ADD &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_SUB &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_MUL &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_DIV &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_MOD &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_EXP &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_CONCAT &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_AND &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_OR &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_XOR &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_SHIFT_LEFT &&
       lex.get_current_operand() != OP_TYPE_ASSIGN_SHIFT_RIGHT))
    {
        xcom.exit_on_code_error("Illegal assignment operator");
    }
    else
    {
        assign_operand = lex.get_current_operand();
    }

    parse_expression();
    read_token(TOKEN_TYPE_DELIM_SEMICOLON);

    //pop the value into _T0
    ii = xicode.add_icode_instruction(current_scope, INSTR_POP);
    xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);

    //if the variable was an array, pop the top of the stack into _T1 for use as the index
    if(is_array)
    {
        ii = xicode.add_icode_instruction(current_scope, INSTR_POP);
        xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_1_symbol_index);
    }

    //generate the I-code for the assignment instruction
    switch(assign_operand)
    {
    // =
    case OP_TYPE_ASSIGN:
        ii = xicode.add_icode_instruction(current_scope, INSTR_MOV);
        break;
    // +=
    case OP_TYPE_ASSIGN_ADD:
        ii = xicode.add_icode_instruction(current_scope, INSTR_ADD);
        break;
    // -=
    case OP_TYPE_ASSIGN_SUB:
        ii = xicode.add_icode_instruction(current_scope, INSTR_SUB);
        break;
    // *=
    case OP_TYPE_ASSIGN_MUL:
        ii = xicode.add_icode_instruction(current_scope, INSTR_MUL);
        break;
    // /=
    case OP_TYPE_ASSIGN_DIV:
        ii = xicode.add_icode_instruction(current_scope, INSTR_DIV);
        break;
    // %=
    case OP_TYPE_ASSIGN_MOD:
        ii = xicode.add_icode_instruction(current_scope, INSTR_MOD);
        break;
    // ^=
    case OP_TYPE_ASSIGN_EXP:
        ii = xicode.add_icode_instruction(current_scope, INSTR_EXP);
        break;
    // $=
    case OP_TYPE_ASSIGN_CONCAT:
        ii = xicode.add_icode_instruction(current_scope, INSTR_CONCAT);
        break;
    // &=
    case OP_TYPE_ASSIGN_AND:
        ii = xicode.add_icode_instruction(current_scope, INSTR_AND);
        break;
    // |=
    case OP_TYPE_ASSIGN_OR:
        ii = xicode.add_icode_instruction(current_scope, INSTR_OR);
        break;
    // #=
    case OP_TYPE_ASSIGN_XOR:
        ii = xicode.add_icode_instruction(current_scope, INSTR_XOR);
        break;
    // <<=
    case OP_TYPE_ASSIGN_SHIFT_LEFT:
        ii = xicode.add_icode_instruction(current_scope, INSTR_SHL);
        break;
    // >>=
    case OP_TYPE_ASSIGN_SHIFT_RIGHT:
        ii = xicode.add_icode_instruction(current_scope, INSTR_SHR);
        break;
    }

    //generate the destination operand
    if(is_array)
    {
        xicode.add_array_index_var_icode_op(current_scope, ii, s->index, xcom.temp_var_1_symbol_index);
    }
    else
    {
        xicode.add_var_icode_op(current_scope, ii, s->index);
    }

    //generate the source
    xicode.add_var_icode_op(current_scope, ii, xcom.temp_var_0_symbol_index);
}

//<Ident>(<Expr>, <Expr>);
void parser::parse_function_call()
{
    function* f = xcom.get_function_by_name(lex.get_current_lexeme());
    int param_count = 0;

    read_token(TOKEN_TYPE_DELIM_OPEN_PAREN);
    while(true)
    {
        if(lex.get_look_ahead_char() != ')')
        {
            parse_expression();

            ++param_count;
            if(!f->is_host_api && param_count > f->param_count)
            {
                xcom.exit_on_code_error("Too many parameters");
            }

            if(lex.get_look_ahead_char() != ')')
            {
                read_token(TOKEN_TYPE_DELIM_COMMA);
            }
        }
        else
        {
            break;
        }
    }

    read_token(TOKEN_TYPE_DELIM_CLOSE_PAREN);

    if(!f->is_host_api && param_count < f->param_count)
    {
        xcom.exit_on_code_error("Too few parameters");
    }

    int instruction = f->is_host_api ? INSTR_CALLHOST : INSTR_CALL;
    int ii = xicode.add_icode_instruction(current_scope, instruction);
    xicode.add_function_icode_op(current_scope, ii, f->index);
}

}//namespace xcomplier
}//namespace xscript
