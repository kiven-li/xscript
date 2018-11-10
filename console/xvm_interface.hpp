#ifndef     __XSCRIPT_XVM_INTERFACE_HPP__
#define     __XSCRIPT_XVM_INTERFACE_HPP__

#include <string>
using std::string;

#define XS_GLOBAL_FUNC              -1// Flags a host API function as being global

enum SCRIPT_LOAD_ERROR_CODE
{
    XS_LOAD_OK = 0,
    XS_LOAD_ERROR_FILE_IO,
    XS_LOAD_ERROR_INVALID_XSE,
    XS_LOAD_ERROR_UNSUPPORTED_VERS,
    XS_LOAD_ERROR_OUT_OF_MEMORY,
    XS_LOAD_ERROR_OUT_OF_THREADS,
};

enum THREAD_PRIORITY
{
    XS_INFINITE_TIMESLICE = -1,//allows a thread to run indefinitely
    XS_THREAD_PRIORITY_USER = 0,
    XS_THREAD_PRIORITY_LOW,
    XS_THREAD_PRIORITY_MED,
    XS_THREAD_PRIORITY_HIGH, 
};

typedef void(*host_api_function_ptr)(int thread_index);//host API function pointer alias 

namespace xscript {
namespace xvm {

class xvm_interface
{
public:
    //------------script interface----------//
    virtual void xvm_init() = 0;
    virtual void xvm_shutdown() = 0;
    
    virtual int xvm_load_script(const char* script_name, int& script_index, int thread_timeslice) = 0;
    virtual void xvm_unload_script(int script_index) = 0;
    virtual void xvm_reset_script(int script_index) = 0;

    virtual void xvm_run_script(int timeslice_duration) = 0;

    virtual void xvm_start_script(int script_index) = 0;
    virtual void xvm_stop_script(int script_index) = 0;
    virtual void xvm_pause_script(int script_index, int duration) = 0;
    virtual void xvm_unpause_script(int script_index) = 0;

    virtual void xvm_pass_int_param(int script_index, int v) = 0;
    virtual void xvm_pass_float_param(int script_index, float v) = 0;
    virtual void xvm_pass_string_param(int script_index, const char* str) = 0;

    virtual int xvm_get_return_as_int(int script_index) = 0;
    virtual float xvm_get_return_as_float(int script_index) = 0;
    virtual string xvm_get_return_as_string(int script_index) = 0;

    virtual void xvm_call_script_function(int script_index, const char* fname) = 0;
    virtual void xvm_invoke_script_function(int script_index, const char* fname) = 0;

    //------------host API interface----------------//
    virtual void xvm_register_host_api(int script_index, const char* fname, host_api_function_ptr fn) = 0;

    virtual int xvm_get_param_as_int(int script_index, int param_index) = 0;
    virtual float xvm_get_param_as_float(int script_index, int param_index) = 0;
    virtual string xvm_get_param_as_string(int script_index, int param_index) = 0;

    virtual void xvm_return_from_host(int script_index, int param_count) = 0;
    virtual void xvm_return_int_from_host(int script_index, int param_count, int v) = 0;
    virtual void xvm_return_float_from_host(int script_index, int param_count, float v) = 0;
    virtual void xvm_return_string_from_host(int script_index, int param_count, char* str) = 0;
};

}//namespace xvm
}//namespace xscript

#endif      //__XSCRIPT_XVM_INTERFACE_HPP__
