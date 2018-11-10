#ifndef     __XSCRIPT_XVM_HPP__
#define     __XSCRIPT_XVM_HPP__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h> 
#include <assert.h>

#include <vector>

#include "xvm_interface.hpp"
#include "../common/instruction.hpp"
#include "../common/utility.hpp"

namespace xscript {
namespace xvm {

//script loading
#define     EXEC_FILE_EXT               ".XSE"
#define     XSE_ID_STRING               "XSE0"
#define     MAJOR_VERSION               0
#define     MINOR_VERSION               8
#define     MAX_THREAD_COUNT            1024//the maximum number of scripts that can be loaded at once.
#define     DEF_STACK_SIZE              1024
#define     MAX_COERCION_STRING_SIZE    64//the maximum allocated space for a string coercion
#define     MAX_HOST_API_SIZE           1024//maximum number of functions in the host API
#define     MAX_FUNC_NAME_SIZE          256

//multithreading
#define     THREAD_PRIORITY_DUR_LOW     20//low-priority thread timeslice
#define     THREAD_PRIORITY_DUR_MED     40
#define     THREAD_PRIORITY_DUR_HIGH    80

enum XVM_THREAD_MODE
{
    THREAD_MODE_MULTI = 0,
    THREAD_MODE_SINGLE,
};

//runtime value
struct xvm_value
{
    int type;
    union
    {
        int int_literal;
        float float_literal;
        //char* string_literal;
        int string_index;
        int stack_index;
        int instruction_index;
        int function_index;
        int host_api_index;
        int reg;
    };
    int offset_index;
};
typedef std::vector<xvm_value> value_vector;

//runtime stack
struct runtime_stack
{
    value_vector elements;
    int size;

    int top;
    int frame;
};

//functions
struct function
{
    int entry_point;
    int param_count;
    int local_data_size;
    int stack_frame_size;
    string name;
};

typedef std::vector<function> function_vector;

//instruction
struct xvm_code
{
    int opcode;
    int opcount;
    value_vector oplist;
};
typedef std::vector<xvm_code> xvm_code_vector;
struct xvm_code_stream
{
    xvm_code_vector codes;
    int current_code;
};

//host API call
typedef std::vector<std::string> string_vector;

//script
struct script
{
    bool is_active;//is this script structure in use

    //header data
    int global_data_size;
    int is_main_function_present;
    int main_function_index;

    //runtime tracking
    bool is_running;
    bool is_paused;
    int pause_end_time;

    //threading
    int timeslice_duration;

    //register file
    xvm_value _RetVal;

    //script data
    function_vector function_table;
    xvm_code_stream code_stream;
    string_vector host_api_table;
    string_vector string_table;

    runtime_stack stack;
};

//host API
struct host_api_function
{
    int is_active;
    int thread_index;

    string name;
    host_api_function_ptr function;
};

//Macros
#define resolve_stack_index(index) (index < 0 ? index += scripts[current_thread].stack.frame : index)
#define is_valid_thread_index(index) (index < 0 || index > MAX_THREAD_COUNT ? false : true)
#define is_thread_active(index) (is_valid_thread_index(index) && scripts[index].is_active ? true : false)

class xvm : public xvm_interface
{
public:
    xvm();
    ~xvm();

    //------------script interface---------------//
    void xvm_init();
    void xvm_shutdown();

    int xvm_load_script(const char* script_name, int& script_index, int thread_timeslice);
    void xvm_unload_script(int script_index);
    void xvm_reset_script(int script_index);

    void xvm_run_script(int timeslice_duration);

    void xvm_start_script(int script_index);
    void xvm_stop_script(int script_index);
    void xvm_pause_script(int script_index, int duration);
    void xvm_unpause_script(int script_index);

    void xvm_pass_int_param(int script_index, int v);
    void xvm_pass_float_param(int script_index, float v);
    void xvm_pass_string_param(int script_index, const char* str);

    int xvm_get_return_as_int(int script_index);
    float xvm_get_return_as_float(int script_index);
    string xvm_get_return_as_string(int script_index);

    void xvm_call_script_function(int script_index, const char* fname);
    void xvm_invoke_script_function(int script_index, const char* fname);

    //------------host API interface---------------//
    void xvm_register_host_api(int script_index, const char* fname, host_api_function_ptr fn);

    int xvm_get_param_as_int(int script_index, int param_index);
    float xvm_get_param_as_float(int script_index, int param_index);
    string xvm_get_param_as_string(int script_index, int param_index);

    void xvm_return_from_host(int script_index, int param_count);
    void xvm_return_int_from_host(int script_index, int param_count, int v);
    void xvm_return_float_from_host(int script_index, int param_count, float v);
    void xvm_return_string_from_host(int script_index, int param_count, char* str);
  
private:      
    //------------operand interface----------------//
    int cast_value_to_int(const xvm_value& v);
    float cast_value_to_float(const xvm_value& v);
    string cast_value_to_string(const xvm_value& v);

    void copy_value(xvm_value* dest, const xvm_value& source);

    int get_operand_type(int index);
    int resolve_operand_stack_index(int index);
    xvm_value resolve_operand_value(int index);
    int resolve_operand_type(int index);
    int resolve_operand_as_int(int index);
    float resolve_operand_as_float(int index);
    string resolve_operand_as_string(int index);
    int resolve_operand_as_instruction_index(int index);
    int resolve_operand_as_function_index(int index);
    string resolve_operand_as_host_api(int index);
    xvm_value* resolve_operand_ptr(int index);

    //------------runtime stack interface-------------//
    xvm_value get_stack_value(int script_index, int index);
    void set_stack_value(int script_index, int index, const xvm_value& v);
    void push(int script_index, const xvm_value& v);
    xvm_value pop(int script_index);
    void push_frame(int script_index, int size);
    void pop_frame(int size);

    //------------function table interface------------//
    int get_function_index_by_name(int script_index, const char* str);
    function get_function(int script_index, int index);

    //------------host API interface-----------------//
    string get_host_api(int index);

    //------------time-------------------------------//
    int get_current_time();

    //------------function---------------------------//
    void call_function(int script_index, int index);

    //------------string table-----------------------//
    int add_string_if_new(const string& str);
    string get_string(int sindex);
private:   
    script scripts[MAX_THREAD_COUNT];
    host_api_function host_apis[MAX_HOST_API_SIZE];

    //threading
    int current_thread;
    int current_thread_mode;    
    int current_thread_active_time;
};

}//namespace xvm
}//namespace xscript

#endif      //__XSCRIPT_XVM_HPP__
