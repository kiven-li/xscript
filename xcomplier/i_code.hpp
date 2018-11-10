#ifndef     __XSCRIPT_XCOMPLIER_I_CODE_HPP__
#define     __XSCRIPT_XCOMPLIER_I_CODE_HPP__

#include "../common/instruction.hpp"
#include <vector>
#include <string>

using std::string;

enum ICODE_NODE_TYPE
{
    ICODE_NODE_INSTR = 0,// An I-code instruction
    ICODE_NODE_SOURCE_LINE,// Source-code annotation
    ICODE_NODE_JUMP_TARGET,// A jump target
};

#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value
#define OP_TYPE_STRING_INDEX        2           // String literal value
#define OP_TYPE_VAR                 3           // Variable
#define OP_TYPE_ARRAY_INDEX_ABS     4           // Array with absolute index
#define OP_TYPE_ARRAY_INDEX_VAR     5           // Array with relative index
#define OP_TYPE_JUMP_TARGET_INDEX   6           // Jump target index
#define OP_TYPE_FUNC_INDEX          7           // Function index
#define OP_TYPE_REG                 9           // Register

namespace xscript {
namespace xcomplier {
class xcomplier;

struct operand
{
    int type;
    union
    {
        int int_literal;
        float float_literal;
        int string_index;
        int symbol_index;
        int jump_target_index;
        int function_index;
        int reg_code;
    };
    int offset;
    int offset_symbol;
};
typedef std::vector<operand> operand_vector;

struct i_code_instruction
{
    int opcode;
    operand_vector operands;
};

struct i_code
{
    int type;

    i_code_instruction instruction;
    string source_line;
    int jump_target_index;
};
typedef std::vector<i_code> i_code_vector;

class x_icode
{
public:
    explicit x_icode(xcomplier& x);
    ~x_icode();

    i_code* get_icode_by_index(int findex, int iindex);

    void add_icode_source_line(int findex, const string& s);

    int add_icode_instruction(int findex, int opcode);
    operand* get_icode_operand_by_index(i_code* in, int op_index);
    void add_icode_operand(int findex, int in_index, const operand& v);

    void add_int_icode_op(int findex, int in_index, int v);
    void add_float_icode_op(int findex, int in_index, float v);
    void add_string_icode_op(int findex, int in_index, int sindex);
    void add_var_icode_op(int findex, int in_index, int symbol_index);
    void add_array_index_abs_icode_op(int findex, int in_index, int array_symbol_index, int offset);
    void add_array_index_var_icode_op(int findex, int in_index, int array_symbol_index, int offset_symbol);
    void add_function_icode_op(int findex, int in_index, int op_findex);
    void add_reg_icode_op(int findex, int in_index, int reg_code);
    void add_jump_target_icode_op(int findex, int in_index, int tindex);

    int get_next_jump_target_index();
    void add_icode_jump_target(int findex, int tindex);

private:
    int current_jump_target_index;
    xcomplier& xcom;
};


}//namespace xcomplier
}//namespace xscript

#endif      //__XSCRIPT_XCOMPLIER_I_CODE_HPP__
