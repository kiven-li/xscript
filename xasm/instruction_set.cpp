#include "instruction_set.hpp"
#include "utility.hpp"
#include <assert.h>

namespace xscript {
namespace xasm {

instruction_set::instruction_set()
{
}

instruction_set::~instruction_set()
{
}

void instruction_set::init_instruction_set()
{
    // Create a temporary index to use with each instruction

    int iindex;

    // The following code makes repeated calls to add_instruction () to add a hardcoded
    // instruction set to the assembler's vocabulary. Each add_instruction () call is
    // followed by zero or more calls to set_operand_type (), whcih set the supported types of
    // a specific operand. The instructions are grouped by family.

    // ---- Main

    // Mov          Destination, Source
    iindex = add_instruction ( "Mov", INSTR_MOV, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // ---- Arithmetic

    // Add         Destination, Source
    iindex = add_instruction ( "Add", INSTR_ADD, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Sub          Destination, Source
    iindex = add_instruction ( "Sub", INSTR_SUB, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Mul          Destination, Source
    iindex = add_instruction ( "Mul", INSTR_MUL, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Div          Destination, Source
    iindex = add_instruction ( "Div", INSTR_DIV, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Mod          Destination, Source
    iindex = add_instruction ( "Mod", INSTR_MOD, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Exp          Destination, Source
    iindex = add_instruction ( "Exp", INSTR_EXP, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Neg          Destination
    iindex = add_instruction ( "Neg", INSTR_NEG, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Inc          Destination
    iindex = add_instruction ( "Inc", INSTR_INC, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Dec          Destination
    iindex = add_instruction ( "Dec", INSTR_DEC, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // ---- Bitwise

    // And          Destination, Source
    iindex = add_instruction ( "And", INSTR_AND, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Or           Destination, Source
    iindex = add_instruction ( "Or", INSTR_OR, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // XOr          Destination, Source
    iindex = add_instruction ( "XOr", INSTR_XOR, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Not          Destination
    iindex = add_instruction ( "Not", INSTR_NOT, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // ShL          Destination, Source
    iindex = add_instruction ( "ShL", INSTR_SHL, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // ShR          Destination, Source
    iindex = add_instruction ( "ShR", INSTR_SHR, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // ---- String Manipulation

    // Concat       String0, String1
    iindex = add_instruction ( "Concat", INSTR_CONCAT, 2 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG |
                                OP_FLAG_TYPE_STRING );

   // GetChar      Destination, Source, Index
    iindex = add_instruction ( "GetChar", INSTR_GETCHAR, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG |
                                OP_FLAG_TYPE_STRING );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG |
                                OP_FLAG_TYPE_INT );

    // SetChar      Destination, Index, Source
    iindex = add_instruction ( "SetChar", INSTR_SETCHAR, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG |
                                OP_FLAG_TYPE_INT );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG |
                                OP_FLAG_TYPE_STRING );

    // ---- Conditional Branching

    // Jmp          Label
    iindex = add_instruction ( "Jmp", INSTR_JMP, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_LINE_LABEL );

    // JE           Op0, Op1, Label
    iindex = add_instruction ( "JE", INSTR_JE, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JNE          Op0, Op1, Label
    iindex = add_instruction ( "JNE", INSTR_JNE, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JG           Op0, Op1, Label
    iindex = add_instruction ( "JG", INSTR_JG, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JL           Op0, Op1, Label
    iindex = add_instruction ( "JL", INSTR_JL, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JGE          Op0, Op1, Label
    iindex = add_instruction ( "JGE", INSTR_JGE, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // JLE           Op0, Op1, Label
    iindex = add_instruction ( "JLE", INSTR_JLE, 3 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 1, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
    set_operand_type ( iindex, 2, OP_FLAG_TYPE_LINE_LABEL );

    // ---- The Stack Interface

    // Push          Source
    iindex = add_instruction ( "Push", INSTR_PUSH, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Pop           Destination
    iindex = add_instruction ( "Pop", INSTR_POP, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // ---- The Function Interface

    // Call          FunctionName
    iindex = add_instruction ( "Call", INSTR_CALL, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_FUNC_NAME );

    // Ret
    iindex = add_instruction ( "Ret", INSTR_RET, 0 );

    // CallHost      FunctionName
    iindex = add_instruction ( "CallHost", INSTR_CALLHOST, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_HOST_API_CALL );

    // ---- Miscellaneous

    // Pause        Duration
    iindex = add_instruction ( "Pause", INSTR_PAUSE, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );

    // Exit         Code
    iindex = add_instruction ( "Exit", INSTR_EXIT, 1 );
    set_operand_type ( iindex, 0, OP_FLAG_TYPE_INT |
                                OP_FLAG_TYPE_FLOAT |
                                OP_FLAG_TYPE_STRING |
                                OP_FLAG_TYPE_MEM_REF |
                                OP_FLAG_TYPE_REG );
}

int instruction_set::add_instruction(const char* mc, int opcode, int opcount)
{
    instruction inn;

    int slen = strlen(mc);
    char MC[slen + 1];
    memcpy(MC, mc, slen);
    MC[slen] = '\0';
    strupr(MC);

    inn.mnemonic = MC;    
    inn.operand_code = opcode;
    inn.operand_count = opcount;
    inn.operand_types.resize(opcount);

    int index = iset.size();
    iset.push_back(inn);

    return index;
}

void instruction_set::set_operand_type(int index, int op_index, operand_type ot)
{
    assert(index < iset.size());
    assert(op_index < iset[index].operand_types.size());
    iset[index].operand_types[op_index] = ot;
}

bool instruction_set::get_instruction_by_mnemonic(const char* mc, instruction* inn)
{
    for(int i = 0; i < iset.size(); ++i)
    {
        if(iset[i].mnemonic == mc)
        {
            *inn = iset[i];
            return true;
        }
    }

    return false;
}

bool instruction_set::is_instruction(const char* mc)
{
    for(int i = 0; i < iset.size(); ++i)
    {
        if(iset[i].mnemonic == mc)
        {
            return true;
        }
    }

    return false;    
}

}//namespace xasm
}//namespace xscript

