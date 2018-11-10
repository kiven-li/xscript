#include "i_code.hpp"
#include "xcomplier.hpp"

namespace xscript {
namespace xcomplier {

x_icode::x_icode(xcomplier& x)
: current_jump_target_index(0),
  xcom(x)
{
}

x_icode::~x_icode()
{
}

i_code* x_icode::get_icode_by_index(int findex, int iindex)
{
    function* f = xcom.get_function_by_index(findex);
    if(f->i_code_stream.size() == 0)
    {
        return NULL;
    }
    if(iindex < 0 || iindex >= f->i_code_stream.size())
    {
        return NULL;
    }

    return &(f->i_code_stream[iindex]);     
}

void x_icode::add_icode_source_line(int findex, const string& s)
{
    function* f = xcom.get_function_by_index(findex);

    i_code icn;
    icn.type = ICODE_NODE_SOURCE_LINE;
    icn.source_line = s;

    f->i_code_stream.push_back(icn);    
}

int x_icode::add_icode_instruction(int findex, int opcode)
{
    function* f = xcom.get_function_by_index(findex);

    i_code icn;
    icn.type = ICODE_NODE_INSTR;
    icn.instruction.opcode = opcode;

    int index = f->i_code_stream.size();
    f->i_code_stream.push_back(icn);

    return index;    
}

operand* x_icode::get_icode_operand_by_index(i_code* in, int op_index)
{
    if(in->instruction.operands.size() == 0)
    {
        return NULL;
    }
    if(op_index < 0 || op_index >= in->instruction.operands.size())
    {
        return NULL;
    }

    return &(in->instruction.operands[op_index]);
}

void x_icode::add_icode_operand(int findex, int in_index, const operand& v)
{
    i_code* in = get_icode_by_index(findex, in_index);

    in->instruction.operands.push_back(v);
}

void x_icode::add_int_icode_op(int findex, int in_index, int v)
{
    operand op;

    op.type = OP_TYPE_INT;
    op.int_literal = v;

    add_icode_operand(findex, in_index, op);
}

void x_icode::add_float_icode_op(int findex, int in_index, float v)
{
    operand op;

    op.type = OP_TYPE_FLOAT;
    op.float_literal = v;

    add_icode_operand(findex, in_index, op);
}

void x_icode::add_string_icode_op(int findex, int in_index, int sindex)
{
    operand op;

    op.type = OP_TYPE_STRING_INDEX;
    op.string_index = sindex;

    add_icode_operand(findex, in_index, op);
}

void x_icode::add_var_icode_op(int findex, int in_index, int symbol_index)
{
    operand op;

    op.type = OP_TYPE_VAR;
    op.symbol_index = symbol_index;

    add_icode_operand(findex, in_index, op);
}

void x_icode::add_array_index_abs_icode_op(int findex, int in_index, int array_symbol_index, int offset)
{
    operand op;

    op.type = OP_TYPE_ARRAY_INDEX_ABS;
    op.symbol_index = array_symbol_index;
    op.offset = offset;

    add_icode_operand(findex, in_index, op);
}

void x_icode::add_array_index_var_icode_op(int findex, int in_index, int array_symbol_index, int offset_symbol)
{
    operand op;

    op.type = OP_TYPE_ARRAY_INDEX_VAR;
    op.symbol_index = array_symbol_index;
    op.offset_symbol = offset_symbol;

    add_icode_operand(findex, in_index, op);
}

void x_icode::add_function_icode_op(int findex, int in_index, int op_findex)
{
    operand op;

    op.type = OP_TYPE_FUNC_INDEX;
    op.function_index = op_findex;

    add_icode_operand(findex, in_index, op);    
}

void x_icode::add_reg_icode_op(int findex, int in_index, int reg_code)
{
    operand op;

    op.type = OP_TYPE_REG;
    op.reg_code = reg_code;

    add_icode_operand(findex, in_index, op); 
}

void x_icode::add_jump_target_icode_op(int findex, int in_index, int tindex)
{
    operand op;

    op.type = OP_TYPE_JUMP_TARGET_INDEX;
    op.jump_target_index = tindex;

    add_icode_operand(findex, in_index, op);
}

int x_icode::get_next_jump_target_index()
{
    return current_jump_target_index++;
}

void x_icode::add_icode_jump_target(int findex, int tindex)
{
    function* f = xcom.get_function_by_index(findex);

    i_code icn;
    icn.type = ICODE_NODE_JUMP_TARGET;
    icn.jump_target_index = tindex;

    f->i_code_stream.push_back(icn);
}

}//namespace xomplier
}//namespace xscript
