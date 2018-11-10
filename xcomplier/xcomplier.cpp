#include "xcomplier.hpp"

namespace xscript {
namespace xcomplier {

xcomplier::xcomplier()
: lex(),
  xicode(*this),
  xparser(*this, lex, xicode),
  cemit(*this)
{
}

xcomplier::~xcomplier()
{
}

void xcomplier::init(const string& sf, const string& of)
{
    source_file_name = sf;
    output_file_name = of;

    xheader.is_main_function_present = false;
    xheader.stack_size = 0;
    xheader.priority_type = PRIORITY_NONE;

    preserve_output_file = false;
    generate_xse = true;
}

void xcomplier::shut_down()
{

}

void xcomplier::complie()
{
    lex.load_source_file(source_file_name);

    printf("Compiling %s...\n\n", source_file_name.c_str());
    complie_source_file();
    print_complie_state();

    if(generate_xse)
    {
        assembly_output_file();
    }

    if(!preserve_output_file)
    {
        remove(output_file_name.c_str());
    }
}

void xcomplier::complie_source_file()
{
    lex.preprocess_source_file();

    temp_var_0_symbol_index = add_symbol(TEMP_VAR_0, 1, SCOPE_GLOBAL, SYMBOL_TYPE_VAR);
    temp_var_1_symbol_index = add_symbol(TEMP_VAR_1, 1, SCOPE_GLOBAL, SYMBOL_TYPE_VAR);

    xparser.parse_source_code();
    cemit.emit_code();
}

void xcomplier::print_complie_state()
{
    //statistics var
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

        if(s.scope == SCOPE_GLOBAL)
        {
            ++global_count;
        }
    }

    //statistics function & instruction
    int instruction_count = 0;
    int host_api_call_count = 0;
    for(int i = 0; i < function_table.size(); ++i)
    {
        function& f = function_table[i];
        ++host_api_call_count;

        instruction_count += f.i_code_stream.size();
    }

    //print    
    printf("%s created successfully!\n\n", output_file_name.c_str());
    printf("Source Lines Processed: %d\n", lex.get_source_line_count());
    printf("            Stack Size: ");
    if(xheader.stack_size)
    {
        printf("%d", xheader.stack_size);
    }
    else
    {
        printf("Default");
    }
    printf("\n");

    printf("              Priority: ");
    switch(xheader.priority_type)
    {
        case PRIORITY_USER:
            printf("%dms Timeslice", xheader.user_priority);
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
        default:
            printf("Default");
            break;
    }
    printf("\n");

    printf("  Instructions Emitted: %d\n", instruction_count);
    printf("             Variables: %d\n", var_count);
    printf("                Arrays: %d\n", array_count);
    printf("               Globals: %d\n", global_count);
    printf("       String Literals: %d\n", string_table.size());
    printf("        Host API Calls: %d\n", host_api_call_count);
    printf("             Functions: %d\n", function_table.size());

    printf("      _Main () Present: ");
    if(xheader.is_main_function_present)
    {
        printf("Yes (Index %d)\n", xheader.main_function_index);
    }
    else
    {
        printf("No\n");
    }
    printf("\n");
}

void xcomplier::assembly_output_file()
{
    char* command_line_param[3];

    command_line_param[0] = (char*)malloc(strlen("XASM") + 1);
    strcpy(command_line_param[0], "XASM");

    command_line_param[1] = (char*)malloc(output_file_name.size());
    strcpy(command_line_param[1], output_file_name.c_str());

    command_line_param[2] = NULL;

    execvp("./xasm", command_line_param);

    free(command_line_param[0]);
    free(command_line_param[1]);
}

function* xcomplier::get_function_by_index(int index)
{
    if(index >= function_table.size() || index < 0)
    {
        return NULL;
    }

    return &(function_table[index]);
}

function* xcomplier::get_function_by_name(const char* fname)
{
    for(int i = 0; i < function_table.size(); ++i)
    {
        if(function_table[i].name == fname)
        {
            return &(function_table[i]);
        }
    }

    return NULL;
}

int xcomplier::add_function(const char* fname, bool is_host_api)
{
    if(get_function_by_name(fname))
    {
        return -1;
    }

    function f;
    f.name = fname;
    f.index = function_table.size();
    f.is_host_api = is_host_api;
    f.param_count = 0;

    if(strcmp(fname, MAIN_FUNC_NAME) == 0)
    {
        xheader.is_main_function_present = true;
        xheader.main_function_index = f.index;
    } 

    function_table.push_back(f);
    return f.index;
}

void xcomplier::set_function_param_count(int index, int pcount)
{
    if(index >= function_table.size() || index < 0)
    {
        return;
    }

    function_table[index].param_count = pcount;
}

symbol* xcomplier::get_symbol_by_index(int i)
{
    if(i >= symbol_table.size() || i < 0)
    {
        return NULL;
    }

    return &(symbol_table[i]);
}

symbol* xcomplier::get_symbol_by_ident(const char* sname, int scope)
{
    for(int i = 0; i < symbol_table.size(); ++i)
    {
        symbol* s = &(symbol_table[i]);
        if(s->name == sname && (s->scope == scope || s->scope == SCOPE_GLOBAL))
        {
            return s;
        }
    }

    return NULL;
}

int xcomplier::get_size_by_ident(const char* sname, int scope)
{
    symbol* s = get_symbol_by_ident(sname, scope);

    return s->size;
}

int xcomplier::add_symbol(const char* sname, int size, int scope, int type)
{
    if(get_symbol_by_ident(sname, scope))
    {
        return -1;
    }

    symbol s;
    s.name = sname;
    s.size = size;
    s.scope = scope;
    s.type = type;
    s.index = symbol_table.size();
    symbol_table.push_back(s);

    return s.index;
}

int xcomplier::add_string(const char* str)
{
    for(int i = 0; i < string_table.size(); ++i)
    {
        if(string_table[i] == str)
        {
            return i;
        }
    }

    int index = string_table.size();
    string_table.push_back(str);

    return index;
}

string xcomplier::get_string_by_index(int i)
{
    if(i >= string_table.size() || i < 0)
    {
        return "";
    }

    return string_table[i];
}

void xcomplier::exit_on_error(const char* emsg)
{
    printf("Fatal Error: %s.\n", emsg);
    exit(1);
}

void xcomplier::exit_on_code_error(const char* emsg)
{
    printf("Error: %s.\n\n", emsg);
    printf("Line: %d\n", lex.get_current_source_line_index());

    string sline = lex.get_current_source_line();

    int last_char_index = sline.size() - 1;
    if(sline[last_char_index] == '\n')
    {
        sline[last_char_index] = '\0';
    }

    for(int i = 0; i < sline.size(); ++i)
    {
        if(sline[i] == '\t')
        {
            sline[i] = ' ';
        }
    }
    printf("%s\n", sline.c_str());

    for(int i = 0; i < lex.get_lexeme_start_index(); ++i)
    {
        printf(" ");
    }
    printf("^\n");

    printf("Could not compile %s.\n", source_file_name.c_str());

    exit(1);
}

}
}
