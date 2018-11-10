#ifndef     __XSCRIPT_INSTRUCTION_SET_HPP__
#define     __XSCRIPT_INSTRUCTION_SET_HPP__

#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <map>

#include "../common/instruction.hpp"

using std::string;

namespace xscript {
namespace xasm {

typedef int operand_type;
typedef std::vector<operand_type> operand_type_vector;
struct instruction
{
    string mnemonic;
    int operand_code;
    int operand_count;
    operand_type_vector operand_types;
};
typedef std::vector<instruction> instruction_vector;

class instruction_set
{
public:
    instruction_set();
    ~instruction_set();

    void init_instruction_set();
    bool get_instruction_by_mnemonic(const char* mc, instruction* inn);
    bool is_instruction(const char* mc);
private:
    int add_instruction(const char* mc, int opcode, int opcount);
    void set_operand_type(int index, int op_index, operand_type ot);

private:
    instruction_vector iset;
};

}//xasm
}//xscript

#endif      //__XSCRIPT_INSTRUCTION_SET_HPP__
