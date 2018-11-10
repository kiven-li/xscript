#ifndef     __XSCRIPT_XSCOPLIER_HPP__
#define     __XSCRIPT_XSCOPLIER_HPP__

#include "lexer.hpp"
#include "parser.hpp"
#include "i_code.hpp"
#include "code_emit.hpp"

#include "globals.hpp"
#include "../common/utility.hpp"
#include <vector>
#include <string>

#define VERSION_MAJOR               0//Major version number
#define VERSION_MINOR               8//Minor version number

#define SOURCE_FILE_EXT             ".XSS"//Extension of a source code file
#define OUTPUT_FILE_EXT             ".XASM"//Extension of an output assembly file

#define MAX_SOURCE_LINE_SIZE        4096// Maximum source line length
#define MAX_IDENT_SIZE              256// Maximum identifier size

enum PRIORITY_ENUM
{
    PRIORITY_NONE = 0,//A priority wasn't specified
    PRIORITY_USER,//User-defined priority
    PRIORITY_LOW,//Low priority
    PRIORITY_MED,//Medium priority
    PRIORITY_HIGH,//High priority
};

#define PRIORITY_LOW_KEYWORD        "Low"//Low priority keyword
#define PRIORITY_MED_KEYWORD        "Med"//Low priority keyword
#define PRIORITY_HIGH_KEYWORD       "High"//Low priority keyword

#define MAIN_FUNC_NAME				"main"//_Main ()'s name

#define REG_CODE_RETVAL             0//_RetVal
    
#define TEMP_VAR_0                  "_T0"//Temporary variable 0
#define TEMP_VAR_1                  "_T1"//Temporary variable 1

#define SCOPE_GLOBAL                0//Global scope

enum SYMBOL_TYPE
{
    SYMBOL_TYPE_VAR = 0,
    SYMBOL_TYPE_PARAM,
};

using std::string;

namespace xscript {
namespace xcomplier {

typedef std::vector<std::string> string_vector;

struct function
{
    int index;
    std::string name;
    bool is_host_api;
    int param_count;
    i_code_vector i_code_stream;
};
typedef std::vector<function> function_vector;

struct symbol
{
    int index;
    std::string name;
    int size;
    int scope;
    int type;
};
typedef std::vector<symbol> symbol_vector;

struct script_header
{
    int stack_size;

    bool is_main_function_present;
    int main_function_index;

    int priority_type;
    int user_priority;
};

class xcomplier
{
public:
    xcomplier();
    ~xcomplier();

    void init(const string& sf, const string& of);
    void shut_down();
    void complie();

    void complie_source_file();
    void print_complie_state();

    void assembly_output_file();

    //function
    function* get_function_by_index(int index);
    function* get_function_by_name(const char* fname);
    int add_function(const char* fname, bool is_host_api);
    void set_function_param_count(int index, int pcount); 

    //symbol
    symbol* get_symbol_by_index(int i);
    symbol* get_symbol_by_ident(const char* sname, int scope);
    int get_size_by_ident(const char* sname, int scope);
    int add_symbol(const char* sname, int size, int scope, int type);

    //string
    int add_string(const char* str);
    string get_string_by_index(int i);

    //error
    void exit_on_error(const char* emsg);
    void exit_on_code_error(const char* emsg);

    //private table
    function_vector function_table;
    symbol_vector symbol_table;
    string_vector string_table;    

    script_header xheader;

    string source_file_name;
    string output_file_name;

    int temp_var_0_symbol_index;
    int temp_var_1_symbol_index;

    bool preserve_output_file;
    bool generate_xse;

    x_icode xicode;//emit_code used.
private:
    lexer lex;
    parser xparser;
    code_emit cemit;
};

}
}

#endif    //__XSCRIPT_XSCOPLIER_HPP__
