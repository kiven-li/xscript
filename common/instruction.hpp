#ifndef     __XSCRIPT_INSTRUCTION_HPP__
#define     __XSCRIPT_INSTRUCTION_HPP__

namespace xscript {

enum INSTRUCTION_CODE
{
    INSTR_MOV = 0,

    INSTR_ADD,
    INSTR_SUB,
    INSTR_MUL,
    INSTR_DIV,
    INSTR_MOD,
    INSTR_EXP,
    INSTR_NEG,
    INSTR_INC,
    INSTR_DEC,

    INSTR_AND,
    INSTR_OR,
    INSTR_XOR,
    INSTR_NOT,
    INSTR_SHL,
    INSTR_SHR,

    INSTR_CONCAT,
    INSTR_GETCHAR,
    INSTR_SETCHAR,

    INSTR_JMP,
    INSTR_JE,
    INSTR_JNE,
    INSTR_JG,
    INSTR_JL,
    INSTR_JGE,
    INSTR_JLE,

    INSTR_PUSH,
    INSTR_POP,

    INSTR_CALL,
    INSTR_RET,
    INSTR_CALLHOST,

    INSTR_PAUSE,
    INSTR_EXIT,
};

static char INSTRUCTION_DESCRIPTION[INSTR_EXIT + 1][10] = 
{
    "MOV",

    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "EXP",
    "NEG",
    "INC",
    "DEC",

    "AND",
    "OR",
    "XOR",
    "NOT",
    "SHL",
    "SHR",

    "CONCAT",
    "GETCHAR",
    "SETCHAR",

    "JMP",
    "JE",
    "JNE",
    "JG",
    "JL",
    "JGE",
    "JLE",

    "PUSH",
    "POP",

    "CALL",
    "RET",
    "CALLHOST",

    "PAUSE",
    "EXIT"
};

//operand type bitfield flags
//The following constants are used as flags into an operand type bit field, hence their values being increasing powers of 2.
#define     OP_FLAG_TYPE_INT        1//Integer literal value
#define     OP_FLAG_TYPE_FLOAT      2//Floating-point literal value
#define     OP_FLAG_TYPE_STRING     4//Integer literal value
#define     OP_FLAG_TYPE_MEM_REF    8//Memory reference(variable or array) index, both absolute and relative
#define     OP_FLAG_TYPE_LINE_LABEL 16//Line label(used for jumps)
#define     OP_FLAG_TYPE_FUNC_NAME  32//Function table index(used for Call)
#define     OP_FLAG_TYPE_HOST_API_CALL  64//Host API Call table index(used for CallHost)
#define     OP_FLAG_TYPE_REG        128//Register

enum OP_TYPE
{
    OP_TYPE_NULL = -1,//uninitialized / null data
    OP_TYPE_INT = 0, //Integer literal value
    OP_TYPE_FLOAT, //Floating-point literal value
    OP_TYPE_STRING_INDEX, //String literal value
    OP_TYPE_ABS_STACK_INDEX, //Absolute array index
    OP_TYPE_REL_STACK_INDEX, //Relative array index
    OP_TYPE_INSTR_INDEX, //Instruction index
    OP_TYPE_FUNC_INDEX, //Function index
    OP_TYPE_HOST_API_CALL_INDEX, //Host API call index
    OP_TYPE_REG, //Register

    OP_TYPE_STACK_BASE_MARKER,//marks a stack base
};

}//xscript

#endif      //__XSCRIPT_INSTRUCTIONHPP__
