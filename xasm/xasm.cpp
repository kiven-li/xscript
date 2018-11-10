#include "xasm.hpp"
#include "utility.hpp"
#include <assert.h>

namespace xscript {
namespace xasm {

xasm::xasm()
:   iset(),
    xlexer(iset)
{
}

xasm::~xasm()
{
}

void xasm::print_logo()
{
    /*printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    printf("┃ xxxxxxxxxxxxxxxxxxxxxxxxx┃\n");
    printf("┃ xx   xxxxxxxxxxxxxxx   xx┃\n");
    printf("┃ xxxx   xxxxxxxxxxx   xxxx┃\n");
    printf("┃ xxxxxx   xxxxxxx   xxxxxx┃\n");
    printf("┃ xxxxxxxx   xxx   xxxxxxxx┃\n");
    printf("┃ xxxxxxxxxx     xxxxxxxxxx┃\n");
    printf("┃ xxxxxxxxxx     xxxxxxxxxx┃\n");
    printf("┃ xxxxxxxx   xxx   xxxxxxxx┃\n");
    printf("┃ xxxxxx   xxxxxxx   xxxxxx┃\n");
    printf("┃ xxxx   xxxxxxxxxxx   xxxx┃\n");
    printf("┃ xx   xxxxxxxxxxxxxxx   xx┃\n");
    printf("┃ xxxxxxxxxxxxxxxxxxxxxxxxx┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
    */
    printf("XASM\n");
    printf("XSCRIPT ASSEMBLER VERSION:%d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    printf("\n");
}

void xasm::init()
{
    iset.init_instruction_set();
}

int xasm::add_string(string_map& stable, const char* str)
{
    int string_index = stable.size();

    string_map::iterator it = stable.find(str);
    if(it != stable.end())
    {
        return it->second;
    }

    stable[str] = string_index;

    return string_index;
}

int xasm::add_function(const char* fname, int entry_point)
{
    if(function_table.count(fname) > 0)
    {
        return -1;
    }

    function fun;
    fun.index = function_table.size();
    fun.name = fname;
    fun.entry_point = entry_point;
    
    function_table[fname] = fun;

    return fun.index;
}

function* xasm::get_function_by_name(const char* fname)
{
    function_map::iterator it = function_table.find(fname);
    if(it != function_table.end())
    {
        return &(it->second);
    }

    return NULL;
}

void xasm::set_function_info(const char* fname, int param_count, int local_data_size)
{
    function_map::iterator it = function_table.find(fname);
    if(it != function_table.end())
    {
        it->second.param_count = param_count;
        it->second.local_data_size = local_data_size;
    }
}

int xasm::add_label(const char* ident, int target_index, int function_index)
{
    if(get_label_by_name(ident, function_index))
    {
        return -1;
    }

    label ll;
    ll.index = label_table.size();
    ll.name = ident;
    ll.target_index = target_index;
    ll.function_index = function_index;

    label_table.push_back(ll);

    return ll.index;
}

label* xasm::get_label_by_name(const char* ident, int function_index)
{
    for(int i = 0; i < label_table.size(); ++i)
    {
        label& ll = label_table[i];
        if(ll.name == ident && ll.function_index == function_index)
        {
            return &(label_table[i]);
        }
    }

    return NULL;
}

int xasm::add_symbol(const char* ident, int size, int stack_index, int function_index)
{
    if(get_symbol_by_name(ident, function_index))
    {
        return -1;
    }

    symbol s;
    s.index = symbol_table.size();
    s.name = ident;
    s.size = size;
    s.stack_index = stack_index;
    s.function_index = function_index;

    symbol_table.push_back(s);

    return s.index;   
}

symbol* xasm::get_symbol_by_name(const char* ident, int function_index)
{
    if(symbol_table.size() == 0)
    {
        return NULL;
    }

    for(int i = 0; i < symbol_table.size(); ++i)
    {
        symbol& s = symbol_table[i];
        if(s.name == ident && (s.function_index == function_index || s.stack_index >= 0))
        {
            return &(symbol_table[i]);
        }
    }

    return NULL;    
}

int xasm::get_stack_index_by_name(const char* ident, int function_index)
{
    symbol* s = get_symbol_by_name(ident, function_index);
    if(s)
    {
        return s->stack_index;
    }

    return 0;
}

int xasm::get_size_by_name(const char* ident, int function_index)
{
    symbol* s = get_symbol_by_name(ident, function_index);
    if(s)
    {
        return s->size;
    }

    return 0;    
}

void xasm::first_pass()
{
    //first pass : add function, add label, check instruction
    
    int code_stream_size = 0;

    //set the current function's flags and variables
    bool is_set_priority_found = false;
    bool is_function_active = false;
    int current_function_index;
    string current_function_name;
    int current_function_param_count = 0;
    int current_function_local_data_size = 0;

    //reset the lexer
    xlexer.reset_lexer();

    //loop trough each line of code
    while(true)
    {
        if(xlexer.read_next_token() == END_OF_TOKEN_STREAM)
        {
            break;
        }

        switch(xlexer.get_current_token())
        {
        //SetStackSize
        case TOKEN_TYPE_SETSTACKSIZE:
        {
            if(is_function_active)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_LOCAL_SETSTACKSIZE);
            }
            if(xscript_header.stack_size > 0)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_MULTIPLE_SETSTACKSIZES);
            }
            if(xlexer.read_next_token() != TOKEN_TYPE_INT)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_STACK_SIZE);
            }

            xscript_header.stack_size = atoi(xlexer.get_current_lexeme());
            break;
        }
        //SetPriority
        case TOKEN_TYPE_SETPRIORITY:
        {
            if(is_function_active)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_LOCAL_SETPRIORITY);
            }
            if(is_set_priority_found)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_MULTIPLE_SETPRIORITIES);
            }

            xlexer.read_next_token();

            switch(xlexer.get_current_token())
            {
            case TOKEN_TYPE_INT:
                xscript_header.user_priority = atoi(xlexer.get_current_lexeme());
                xscript_header.stack_size = PRIORITY_USER;
                break;
            case TOKEN_TYPE_IDENT:
                if(strcmp(xlexer.get_current_lexeme(), PRIORITY_LOW_KEYWORD) == 0)
                {
                    xscript_header.priority_type = PRIORITY_LOW;
                }
                else if(strcmp(xlexer.get_current_lexeme(), PRIORITY_MED_KEYWORD) == 0)
                {
                    xscript_header.priority_type = PRIORITY_MED;
                }
                else if(strcmp(xlexer.get_current_lexeme(), PRIORITY_HIGH_KEYWORD) == 0)
                {
                    xscript_header.priority_type = PRIORITY_HIGH;
                }
                else
                {
                    xlexer.exit_on_code_error(ERROR_MSSG_INVALID_PRIORITY);
                }
                break;
            default:
                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_PRIORITY);
            }

            is_set_priority_found = true;
            break;
        }
        //Var/Var []
        case TOKEN_TYPE_VAR:
        {
            if(xlexer.read_next_token() != TOKEN_TYPE_IDENT)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_IDENT_EXPECTED);
            }

            string identifier = xlexer.get_current_lexeme();

            //detemine var size
            int size = 1;
            if(xlexer.read_look_ahead_char() == '[')
            {
                if(xlexer.read_next_token() != TOKEN_TYPE_OPEN_BRACKET)
                {
                    xlexer.exit_on_char_expected_error('[');
                }
                if(xlexer.read_next_token() != TOKEN_TYPE_INT)
                {
                    xlexer.exit_on_code_error(ERROR_MSSG_INVALID_ARRAY_SIZE);
                }

                size = atoi(xlexer.get_current_lexeme());
                if(size <= 0)
                {
                    xlexer.exit_on_code_error(ERROR_MSSG_INVALID_ARRAY_SIZE);
                }

                if(xlexer.read_next_token() != TOKEN_TYPE_CLOSE_BRACKET)
                {
                    xlexer.exit_on_char_expected_error(']');
                }
            }

            //determine the var's index into the stack
            //if the variable is local, then its stack index is always the local data size+2 subtracted from zero
            int stack_index;
            if(is_function_active)
            {
                stack_index = -(current_function_local_data_size + 2);
            }
            else
            {
                stack_index = xscript_header.global_data_size;
            }

            //add the symbol to the table
            if(add_symbol(identifier.c_str(), size, stack_index, current_function_index) == -1)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_IDENT_REDEFINITION);
            }

            //depending on the scope,increment either the local or global data size by the size of the variable
            if(is_function_active)
            {
                current_function_local_data_size += size;
            }
            else
            {
                xscript_header.global_data_size += size;
            }

            break;
        }
        //Func
        case TOKEN_TYPE_FUNC:
        {
            if(is_function_active)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_NESTED_FUNC);
            }
            if(xlexer.read_next_token() != TOKEN_TYPE_IDENT)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_IDENT_EXPECTED);
            }

            string function_name = xlexer.get_current_lexeme();
            int entry_point = code_stream_size;

            //adding it to the function table
            int function_index = add_function(function_name.c_str(), entry_point);
            if(function_index == -1)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_FUNC_REDEFINITION);
            }

            //is this the _Main() function?
            if(function_name == MAIN_FUNC_NAME)
            {
                xscript_header.is_main_function_present = 1;
                xscript_header.main_function_index = function_index;
            }

            //initialize function tracking variables
            is_function_active = true;
            current_function_name = function_name;
            current_function_index = function_index;
            current_function_param_count = 0;
            current_function_local_data_size = 0;

            //read any number of line breaks until the opening brace is found
            while(xlexer.read_next_token() == TOKEN_TYPE_NEWLINE);

            //make sure the lexeme was an opening brace
            if(xlexer.get_current_token() != TOKEN_TYPE_OPEN_BRACE)
            {
                xlexer.exit_on_char_expected_error('{');
            }

            //all functions are automatically appended with Ret, so increment the required size of the instruction stream
            ++code_stream_size;

            break;
        }
        //closeing bracket
        case TOKEN_TYPE_CLOSE_BRACE:
        {
            if(!is_function_active)
            {
                xlexer.exit_on_char_expected_error('}');
            }

            //set the fields we've collected
            set_function_info(current_function_name.c_str(), current_function_param_count, current_function_local_data_size);

            //close the function
            is_function_active = false;

            break;
        }
        //Param
        case TOKEN_TYPE_PARAM:
        {
            if(!is_function_active)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_GLOBAL_PARAM);
            }
            if(current_function_name == MAIN_FUNC_NAME)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_MAIN_PARAM);
            }
            if(xlexer.read_next_token() != TOKEN_TYPE_IDENT)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_IDENT_EXPECTED);
            }

            ++current_function_param_count;

            break;
        }
        //instructions
        case TOKEN_TYPE_INSTR:
        {
            if(!is_function_active)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_GLOBAL_INSTR);
            }

            ++code_stream_size;
            break;
        }
        //identifiers(line labels)
        case TOKEN_TYPE_IDENT:
        {
            if(xlexer.read_look_ahead_char() != ':')
            {
                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_INSTR);
            }
            if(!is_function_active)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_GLOBAL_LINE_LABEL);
            }

            string identifier = xlexer.get_current_lexeme();

            //the target instruction is always the value of the current insruction count, which is the current size - 1
            int target_index = code_stream_size - 1;

            //adding the label to the label table
            if(add_label(identifier.c_str(), target_index, current_function_index) == -1)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_LINE_LABEL_REDEFINITION);
            }

            break;
        }
        default:
            if(xlexer.get_current_token() != TOKEN_TYPE_NEWLINE)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_INPUT);
            }
        }//switch(xlexer.current_token)

        if(!xlexer.skip_to_next_line())
        {
            break;
        }
    }

    //resize the assembled instruction stream array
    code_stream.resize(code_stream_size);    
}

void xasm::second_pass()
{
    function* current_fuction;
    bool is_function_active = false;
    int current_function_index;
    int current_function_param_count = 0;
    int current_code_index = 0;

    //create an instruction definition structure to hold instruction information when dealing with instructions.
    instruction current_instruction;

    xlexer.reset_lexer();

    while(true)
    {
        if(xlexer.read_next_token() == END_OF_TOKEN_STREAM)
        {
            break;
        }

        switch(xlexer.get_current_token())
        {
        //Func
        case TOKEN_TYPE_FUNC:
        {
            xlexer.read_next_token();
            current_fuction = get_function_by_name(xlexer.get_current_lexeme());
            is_function_active = true;
            current_function_param_count = 0;
            current_function_index = current_fuction->index;

            while(xlexer.read_next_token() == TOKEN_TYPE_NEWLINE);

            break;
        }
        //closing brace
        case TOKEN_TYPE_CLOSE_BRACE:
        {
            is_function_active = false;

            if(current_fuction->name == MAIN_FUNC_NAME)
            {
                code_stream[current_code_index].operand_code = INSTR_EXIT;
                code_stream[current_code_index].operand_count = 1;

                //now set the return code
                operand op;
                op.type = OP_TYPE_INT;
                op.int_literal = 0;
                code_stream[current_code_index].operands.push_back(op);
            }
            else
            {
                code_stream[current_code_index].operand_code = INSTR_RET;
                code_stream[current_code_index].operand_count = 0;
            }

            ++current_code_index;
            break;
        }
        case TOKEN_TYPE_PARAM:
        {
            if(xlexer.read_next_token() != TOKEN_TYPE_IDENT)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_IDENT_EXPECTED);
            }

            string identifier = xlexer.get_current_lexeme();

            //calculate the parameter's stack index
            int stack_index = -(current_fuction->local_data_size + 2 + (current_function_param_count + 1));

            //add the parameter to the symbol table
            if(add_symbol(identifier.c_str(), 1, stack_index, current_function_index) == -1)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_IDENT_REDEFINITION);
            }

            ++current_function_param_count;
            break;
        }
        case TOKEN_TYPE_INSTR:
        {
            //get the instruction's info using the current lexeme(the mnemonic)
            get_instruction_by_mnemonic(xlexer.get_current_lexeme(), &current_instruction);

            code_stream[current_code_index].operand_code = current_instruction.operand_code;
            code_stream[current_code_index].operand_count = current_instruction.operand_count;

            code_stream[current_code_index].operands.resize(current_instruction.operand_count);
            for(int i = 0; i < current_instruction.operand_count; ++i)
            {
                operand_type current_operand_type = current_instruction.operand_types[i];
                
                token init_operand_token = xlexer.read_next_token();
                switch(init_operand_token)
                {
                case TOKEN_TYPE_INT:
                    if(current_operand_type & OP_FLAG_TYPE_INT)
                    {
                        code_stream[current_code_index].operands[i].type = OP_TYPE_INT;
                        code_stream[current_code_index].operands[i].int_literal = atoi(xlexer.get_current_lexeme());
                    }
                    else
                    {
                        xlexer.exit_on_code_error(ERROR_MSSG_INVALID_OP);
                    }
                    break;
                case TOKEN_TYPE_FLOAT:
                    if(current_operand_type & OP_FLAG_TYPE_FLOAT)
                    {
                        code_stream[current_code_index].operands[i].type = OP_TYPE_FLOAT;
                        code_stream[current_code_index].operands[i].int_literal = (float)atof(xlexer.get_current_lexeme());
                    }
                    else
                    {
                        xlexer.exit_on_code_error(ERROR_MSSG_INVALID_OP);
                    }
                    break;
                case TOKEN_TYPE_STRING:
                {
                    if(current_operand_type & OP_FLAG_TYPE_STRING)
                    {
                        string str = xlexer.get_current_lexeme();
                        int string_index = add_string(string_table, str.c_str());

                        //if(xlexer.read_next_token() != TOKEN_TYPE_QUOTE)
                        //{
                        //    xlexer.exit_on_char_expected_error('\\');
                        //}

                        code_stream[current_code_index].operands[i].type = OP_TYPE_STRING_INDEX;
                        code_stream[current_code_index].operands[i].string_table_index = string_index;
                    }
                    else
                    {
                        xlexer.exit_on_code_error(ERROR_MSSG_INVALID_OP);  
                    }

                    break;
                }
                //_RetVal
                case TOKEN_TYPE_REG_RETVAL:
                    if(current_operand_type & OP_FLAG_TYPE_REG)
                    {
                        code_stream[current_code_index].operands[i].type = OP_TYPE_REG;
                        code_stream[current_code_index].operands[i].reg = 0;
                    }
                    else
                    {
                        xlexer.exit_on_code_error(ERROR_MSSG_INVALID_OP);
                    }
                    break;
                //identifiers -variables/array -line label -function name - host api
                case TOKEN_TYPE_IDENT:
                {
                    //parse a memory reference a variable or array index
                    if(current_operand_type & OP_FLAG_TYPE_MEM_REF)
                    {
                        string identifier = xlexer.get_current_lexeme();
                        if(!get_symbol_by_name(identifier.c_str(), current_function_index))
                        {
                            xlexer.exit_on_code_error(ERROR_MSSG_UNDEFINED_IDENT);
                        }

                        int base_index = get_stack_index_by_name(identifier.c_str(), current_function_index);
                        if(xlexer.read_look_ahead_char() != '[')
                        {
                            if(get_size_by_name(identifier.c_str(), current_function_index) > 1)
                            {
                                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED);
                            }

                            code_stream[current_code_index].operands[i].type = OP_TYPE_ABS_STACK_INDEX;
                            code_stream[current_code_index].operands[i].int_literal = base_index;
                        }
                        else
                        {
                            if(get_size_by_name(identifier.c_str(), current_function_index) == 1)
                            {
                                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_ARRAY);
                            }
                            if(xlexer.read_next_token() != TOKEN_TYPE_OPEN_BRACKET)
                            {
                                xlexer.exit_on_char_expected_error('[');
                            }

                            //the next token is the index, be it an integer or variable
                            token index_token = xlexer.read_next_token();
                            if(index_token == TOKEN_TYPE_INT)
                            {
                                int offset_index = atoi(xlexer.get_current_lexeme());

                                code_stream[current_code_index].operands[i].type = OP_TYPE_ABS_STACK_INDEX;
                                code_stream[current_code_index].operands[i].int_literal = base_index + offset_index;
                            }
                            else if(index_token == TOKEN_TYPE_IDENT)
                            {
                                string index_identifier = xlexer.get_current_lexeme();
                                if(!get_symbol_by_name(index_identifier.c_str(), current_function_index))
                                {
                                    xlexer.exit_on_code_error(ERROR_MSSG_UNDEFINED_IDENT);
                                }
                                if(get_size_by_name(index_identifier.c_str(), current_function_index) > 1)
                                {
                                    xlexer.exit_on_code_error(ERROR_MSSG_INVALID_ARRAY_INDEX);
                                }

                                int offset_index = get_stack_index_by_name(index_identifier.c_str(), current_function_index);
                                code_stream[current_code_index].operands[i].type = OP_TYPE_REL_STACK_INDEX;
                                code_stream[current_code_index].operands[i].stack_index = base_index;
                                code_stream[current_code_index].operands[i].offset_index = offset_index;
                            }
                            else
                            {
                                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_ARRAY_INDEX);
                            }

                            if(xlexer.read_next_token() != TOKEN_TYPE_CLOSE_BRACKET)
                            {
                                xlexer.exit_on_char_expected_error(']');
                            }
                        }
                    }
                    //parse line label
                    if(current_operand_type & OP_FLAG_TYPE_LINE_LABEL)
                    {
                        string label_indentifier = xlexer.get_current_lexeme();
                        label* ll = get_label_by_name(label_indentifier.c_str(), current_function_index);

                        if(!ll)
                        {
                            xlexer.exit_on_code_error(ERROR_MSSG_UNDEFINED_LINE_LABEL);
                        }

                        code_stream[current_code_index].operands[i].type = OP_TYPE_INSTR_INDEX;
                        code_stream[current_code_index].operands[i].instruction_index = ll->target_index;
                    }
                    //parse a function name
                    if(current_operand_type & OP_FLAG_TYPE_FUNC_NAME)
                    {
                        string function_name = xlexer.get_current_lexeme();
                        function* f = get_function_by_name(function_name.c_str());
                        if(!f)
                        {
                            xlexer.exit_on_code_error(ERROR_MSSG_UNDEFINED_FUNC);
                        }

                        code_stream[current_code_index].operands[i].type = OP_TYPE_FUNC_INDEX;
                        code_stream[current_code_index].operands[i].instruction_index = f->index;
                    }
                    //parse a host api
                    if(current_operand_type & OP_FLAG_TYPE_HOST_API_CALL)
                    {
                        string host_api_call = xlexer.get_current_lexeme();
                        int hindex = add_string(host_api_table, host_api_call.c_str());

                        code_stream[current_code_index].operands[i].type = OP_TYPE_HOST_API_CALL_INDEX;
                        code_stream[current_code_index].operands[i].host_api_index = hindex;
                    }
                    
                    break;
                }
                default:
                    xlexer.exit_on_code_error(ERROR_MSSG_INVALID_OP);
                    break;
                }

                if(i < current_instruction.operand_count - 1)
                {
                    if(xlexer.read_next_token() != TOKEN_TYPE_COMMA)
                    {
                        xlexer.exit_on_char_expected_error(',');
                    }
                }
            }

            //make sure there's no extranous stuff ahead
            if(xlexer.read_next_token() != TOKEN_TYPE_NEWLINE)
            {
                xlexer.exit_on_code_error(ERROR_MSSG_INVALID_INPUT);
            }

            //move along to the next instruction in the stream
            ++current_code_index;

            break;
        }
        }

        //skip to the next line
        if(!xlexer.skip_to_next_line())
        {
            break;
        }
    }
}

void xasm::assembly_source_file()
{
    xscript_header.stack_size = 0;
    xscript_header.is_main_function_present = 0;
    xscript_header.global_data_size = 0;
    xscript_header.priority_type = 0;
    xscript_header.user_priority = 0;

    first_pass();
    
    second_pass();
}

void xasm::print_assembly_status()
{
    int var_count = 0;
    int array_count = 0;
    int global_count = 0;

    for(int i = 0; i < symbol_table.size(); ++i)
    {
        symbol& s = symbol_table[i];
        if(s.size > 1)
        {
            ++array_count;    
        }        
        else 
        {
            ++var_count;
        }

        if(s.stack_index > 0) ++global_count;
    }

    printf("%s created successfully!\n\n", execute_file_name.c_str());
    printf("Source Lines Processed: %d\n", xlexer.get_souce_code_line_count());

    printf("            Stack Size: ");
    if(xscript_header.stack_size)
        printf("%d", xscript_header.stack_size);
    else
        printf("Default");

    printf("\n");
    printf("              Priority: ");
    switch(xscript_header.priority_type)
    {
        case PRIORITY_USER:
            printf("%dms Timeslice", xscript_header.user_priority);
            break;

        case PRIORITY_LOW:
            printf(PRIORITY_LOW_KEYWORD);
            break;

        case PRIORITY_MED:
            printf(PRIORITY_MED_KEYWORD);
            break;

        case PRIORITY_HIGH:
            printf(PRIORITY_HIGH_KEYWORD);
            break;
    }    

    printf("\n");
    printf("Instructions Assembled: %d\n", code_stream.size());
    printf("             Variables: %d\n", var_count);
    printf("                Arrays: %d\n", array_count);
    printf("               Globals: %d\n", global_count);
    printf("       String Literals: %d\n", string_table.size());
    printf("                Labels: %d\n", label_table.size());
    printf("        Host API Calls: %d\n", host_api_table.size());
    printf("             Functions: %d\n", function_table.size());

    printf("      _Main () Present: ");
    if(xscript_header.is_main_function_present)
        printf("Yes (Index %d)\n", xscript_header.main_function_index);
    else
        printf("No\n");    
}

void xasm::dump_code_stream()
{
    printf("--------------------START DUMP----------------\n");
    for(int i = 0; i < code_stream.size(); ++i)
    {
        printf("operand_code=%d, operand_count=%d\n", code_stream[i].operand_code, code_stream[i].operand_count);
        for(int m = 0; m < code_stream[i].operands.size(); ++m)
        {
            printf("\t[operand.type=%d, operand.value=%d, operand.offset_index=%d]\n", code_stream[i].operands[m].type, code_stream[i].operands[m].int_literal, code_stream[i].operands[m].offset_index);
        }
        printf("\n");
    }
    printf("--------------------START DUMP----------------\n");
}

void xasm::build_xse()
{
    //dump_code_stream();

    FILE* execute_file;
    if(!(execute_file = fopen(execute_file_name.c_str(), "wb")))
    {
        xlexer.exit_on_error("Could not open executable file for output");
    }

    //-------------write head-------------//
    //write the ID string(4 bytes)
    fwrite(XSE_ID_STRING, 4, 1, execute_file);

    //write the version(1 byte for each component, 2 total)
    char version_major = VERSION_MAJOR;
    char version_minor = VERSION_MINOR;
    fwrite(&version_major, 1, 1, execute_file);
    fwrite(&version_minor, 1, 1, execute_file);

    //write the stack size(4 bytes)
    fwrite(&xscript_header.stack_size, 4, 1, execute_file);

    //write the global data size(4 bytes)
    fwrite(&xscript_header.global_data_size, 4, 1, execute_file);

    //write the _Main() flag(1 byte)
    char is_main_present = xscript_header.is_main_function_present;
    fwrite(&is_main_present, 1, 1, execute_file);

    //write the _Main() function index(4 bytes)
    fwrite(&xscript_header.main_function_index, 4, 1, execute_file);

    //write the priority type(1 byte)
    char priority_type = xscript_header.priority_type;
    fwrite(&priority_type, 1, 1, execute_file);

    //write the user-defined priority(4 byte)
    fwrite(&xscript_header.user_priority, 4, 1, execute_file);

    //---------------write instruction stream-----------------//
    //write instruction count (4 bytes)
    int code_stream_size = code_stream.size();
    fwrite(&code_stream_size, 4, 1, execute_file);

    //write instructions
    for(int i = 0; i < code_stream_size; ++i)
    {
        //write the opcode(2 bytes)
        short opcode = code_stream[i].operand_code;
        fwrite(&opcode, 2, 1, execute_file);

        //write the operand count(1 byte)
        char opcount = code_stream[i].operand_count;
        fwrite(&opcount, 1, 1, execute_file);

        //write operand list
        for(int m = 0; m < opcount; ++m)
        {
            operand current_operand = code_stream[i].operands[m];
            
            //write operand type
            char op_type = current_operand.type;
            fwrite(&op_type, 1, 1, execute_file);

            //write the operand depending on its type
            switch(current_operand.type)
            {
            case OP_TYPE_INT:
                fwrite(&current_operand.int_literal, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_FLOAT:
                fwrite(&current_operand.float_literal, sizeof(float), 1, execute_file);
                break;
            case OP_TYPE_STRING_INDEX:
                fwrite(&current_operand.string_table_index, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_INSTR_INDEX:
                fwrite(&current_operand.instruction_index, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_ABS_STACK_INDEX:
                fwrite(&current_operand.stack_index, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_REL_STACK_INDEX:
                fwrite(&current_operand.stack_index, sizeof(int), 1, execute_file);
                fwrite(&current_operand.offset_index, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_FUNC_INDEX:
                fwrite(&current_operand.function_index, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_HOST_API_CALL_INDEX:
                fwrite(&current_operand.host_api_index, sizeof(int), 1, execute_file);
                break;
            case OP_TYPE_REG:
                fwrite(&current_operand.reg, sizeof(int), 1, execute_file);
                break;
            }
        }
    }

    //----------------string table--------------//
    //write out the string count(4 bytes)
    int string_table_size = string_table.size();
    fwrite(&string_table_size, 4, 1, execute_file);

    //write string by string_index order
    std::vector<string> string_vector;
    string_vector.resize(string_table.size());
    for(string_map::iterator it = string_table.begin(); it != string_table.end(); ++it)
    {
        assert(it->second < string_vector.size());
        string_vector[it->second] = it->first;
    }
    for(int i = 0; i < string_vector.size(); ++i)
    {
        //write the string length(4 bytes), followd by the string data
        int slen = string_vector[i].size();
        fwrite(&slen, 4, 1, execute_file);
        fwrite(string_vector[i].c_str(), slen, 1, execute_file);
    }

    //----------------function table--------------//
    //write out the function count(4 bytes)
    int function_table_size = function_table.size();
    fwrite(&function_table_size, 4, 1, execute_file);

    //write function by function_index order
    std::vector<function> function_vector;
    function_vector.resize(function_table.size());
    for(function_map::iterator it = function_table.begin(); it != function_table.end(); ++it)
    {
        assert(it->second.index < function_vector.size());
        function_vector[it->second.index] = it->second;
    }
    for(int i = 0; i < function_vector.size(); ++i)
    {
        function& f = function_vector[i];

        //write the entry point(4 bytes)
        fwrite(&(f.entry_point), 4, 1, execute_file);

        //write the parameter count(1 byte)
        char param_count = f.param_count;
        fwrite(&param_count, 1, 1, execute_file);

        //write the local data size(4 bytes)
        fwrite(&(f.local_data_size), 4, 1, execute_file);

        //write the function name length(1 byte)
        char function_name_length = f.name.size();
        fwrite(&function_name_length, 1, 1, execute_file);

        //write the function name(N bytes)
        fwrite(f.name.c_str(), f.name.size(), 1, execute_file);
    }

    //----------------host api table--------------//
    //write out the call count(4 bytes)
    int host_api_table_size = host_api_table.size();
    fwrite(&host_api_table_size, 4, 1, execute_file);

    //write host api by host_api_index order
    std::vector<string> host_api_vector;
    host_api_vector.resize(host_api_table.size());
    for(string_map::iterator it = host_api_table.begin(); it != host_api_table.end(); ++it)
    {
        assert(it->second < host_api_vector.size());
        host_api_vector[it->second] = it->first;
    }
    for(int i = 0; i < host_api_vector.size(); ++i)
    {
        char slen = static_cast<char>(host_api_vector[i].size());

        //write the length(1 byte), followed by the string data
        fwrite(&slen, 1, 1, execute_file);
        fwrite(host_api_vector[i].c_str(), slen, 1, execute_file);
    }

    //close file
    fclose(execute_file);
}

void xasm::complier(const string& file_name)
{    
    //set source file name
    source_file_name = file_name;

    //set execute file name
    int extension_offset = strrchr(source_file_name.c_str(), '.') - source_file_name.c_str();
    string main_name(source_file_name.c_str(), extension_offset);
    execute_file_name = main_name + EXEC_FILE_EXT;
    
    //print logo
    print_logo();

    //init complier
    init();

    //load the source file into memory
    xlexer.load_source_file(source_file_name);

    //assemble the source file
    printf ("assembling %s...\n\n", source_file_name.c_str());
    assembly_source_file();

    //dump the assembled executable to an .XSE file
    build_xse();

    //print out assembly statistics
    print_assembly_status(); 
}

}//namespace xasm
}//namespace xscript

