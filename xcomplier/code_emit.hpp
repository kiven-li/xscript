#ifndef     __XSCRIPT_XCOMPLIER_CODE_EMIT_HPP__
#define     __XSCRIPT_XCOMPLIER_CODE_EMIT_HPP__

#include <vector>

#define     TAB_STOP_WIDTH      8

namespace xscript {
namespace xcomplier {

class function;
class xcomplier;

class code_emit
{
public:
    explicit code_emit(xcomplier& x);
    ~code_emit();

    void emit_code();

private:
    void emit_header();
    void emit_directives();
    void emit_scope_symbol(int scope, int type);
    void emit_func(function* f);

private:
    FILE* output_file;
    xcomplier& xcom;
};

}//namespace xcomplier
}//namespace xscript

#endif      //__XSCRIPT_XCOMPLIER_CODE_EMIT_HPP__