#ifndef     __XSCRIPT_XASM_HPP__
#define     __XSCRIPT_XASM_HPP__

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "instruction_set.hpp"
#include "lexer.hpp"
#include "error_code.hpp"

using std::string;

namespace xscript {
namespace xasm {

//file extension name
#define     SOURCE_FILE_EXT     ".XASM"//extension of a source code file
#define     EXEC_FILE_EXT       ".XSE"//extension of an executable code file

//version
#define     XSE_ID_STRING       "XSE0"//written to the file to state it's validity
#define     VERSION_MAJOR       0
#define     VERSION_MINOR       8

//main function name
#define     MAIN_FUNC_NAME  "MAIN"//_Main()'s name

//priority type
#define PRIORITY_LOW_KEYWORD        "LOW"       // Low priority keyword
#define PRIORITY_MED_KEYWORD        "MED"       // Low priority keyword
#define PRIORITY_HIGH_KEYWORD       "HIGH"      // Low priority keyword

enum priority_types
{
    PRIORITY_USER = 0,// User-defined priority
    PRIORITY_LOW,// Low priority
    PRIORITY_MED,// Medium priority
    PRIORITY_HIGH,// High priority    
};

//instruction stream
struct operand
{
    int type;
    union
    {
        int int_literal;
        float float_literal;
        int string_table_index;
        int stack_index;
        int instruction_index;
        int function_index;
        int host_api_index;
        int reg;
    };
    int offset_index;
};
typedef std::vector<operand> operand_vector;

struct code
{
    int operand_code;
    int operand_count;
    operand_vector operands;
};
typedef std::vector<code> code_vector;

//script header
struct script_header
{
    int stack_size;
    int global_data_size;
    int is_main_function_present;
    int main_function_index;

    int priority_type;//the thread priority type
    int user_priority;//the user-defined priority(if any)
};

//tables
struct function
{
    int index;
    string name;
    int entry_point;
    int param_count;
    int local_data_size;
};

struct label
{
    int index;
    string name;
    int target_index;
    int function_index;
};

struct symbol
{
    int index;
    string name;
    int size;
    int stack_index;
    int function_index;
};

typedef std::vector<string> string_vector;
typedef std::map<string, int> string_map;
typedef std::map<string, function> function_map;
typedef std::vector<label> label_vector;
typedef std::vector<symbol> symbol_vector;

class xasm
{
public:
    xasm();
    ~xasm();

    void complier(const string& file_name);

    //misc
    void print_logo();

    //assembly
    void init();

    void first_pass();
    void second_pass();
    void assembly_source_file();
    void build_xse();

    void print_assembly_status();
    void dump_code_stream();

    //instruction
    int get_instruction_by_mnemonic(const char* mc, instruction* inn) { return iset.get_instruction_by_mnemonic(mc, inn); }
    bool is_instruction(const char* mc) { return iset.is_instruction(mc); }

    //tables
    int add_string(string_map& stable, const char* str);

    int add_function(const char* fname, int entry_point);
    function* get_function_by_name(const char* fname);
    void set_function_info(const char* fname, int param_count, int local_data_size);

    int add_label(const char* ident, int target_index, int function_index);
    label* get_label_by_name(const char* ident, int function_index);

    int add_symbol(const char* ident, int size, int stack_index, int function_index);
    symbol* get_symbol_by_name(const char* ident, int function_index);
    int get_stack_index_by_name(const char* ident, int function_index);
    int get_size_by_name(const char* ident, int function_index);

private:
    //instruction set
    instruction_set iset;

    //lexer
    lexer xlexer;

    //file name
    string source_file_name;
    string execute_file_name;

    //script header
    script_header xscript_header;

    //instruction stream
    code_vector code_stream;

    //table
    string_map string_table;
    string_map host_api_table;
    function_map function_table;
    label_vector label_table;
    symbol_vector symbol_table;
};

}//xasm
}//xscript

#endif      //__XSCRIPT_XASM_HPP__
