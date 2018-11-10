#include "xcomplier.hpp"
#include "code_emit.hpp"
#include "i_code.hpp"

namespace xscript {
namespace xcomplier {

char mnemonics_table[][12] =
{
    "Mov",
    "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
    "And", "Or", "XOr", "Not", "ShL", "ShR",
    "Concat", "GetChar", "SetChar",
    "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
    "Push", "Pop",
    "Call", "Ret", "CallHost",
    "Pause", "Exit"
};

code_emit::code_emit(xcomplier& x)
: output_file(NULL),
  xcom(x)
{
}

code_emit::~code_emit()
{
}

void code_emit::emit_header()
{
    time_t current_time_ms = time(NULL);
    struct tm* curremt_time = localtime(&current_time_ms);

    //emit the filename
    fprintf(output_file, "; %s\n\n", xcom.output_file_name.c_str());

    //emit the header
    fprintf(output_file, ";     Source File: %s\n", xcom.source_file_name.c_str());
    fprintf(output_file, "; XSCRIPT Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    fprintf(output_file, ";       Timestamp: %s\n", asctime (curremt_time));
}

void code_emit::emit_directives()
{
    bool is_add_new_line = false;

    if(xcom.xheader.stack_size > 0)
    {
        fprintf(output_file, "\tSetStackSize %d\n", xcom.xheader.stack_size);
        is_add_new_line = true;
    }

    if(xcom.xheader.priority_type != PRIORITY_NONE)
    {
        fprintf(output_file, "\tSetPriority ");
        switch(xcom.xheader.priority_type)
        {
        case PRIORITY_LOW:
            fprintf(output_file, PRIORITY_LOW_KEYWORD);
            break;
        case PRIORITY_MED:
            fprintf(output_file, PRIORITY_MED_KEYWORD);
            break;
        case PRIORITY_HIGH:
            fprintf(output_file, PRIORITY_HIGH_KEYWORD);
            break;
        case PRIORITY_USER:
            fprintf(output_file, "%d", xcom.xheader.user_priority);
            break;  
        }
        fprintf(output_file, "\n");
        is_add_new_line = true;
    }

    if(is_add_new_line)
    {
        fprintf(output_file, "\n");
    }  
}

void code_emit::emit_scope_symbol(int scope, int type) 
{
    bool is_add_new_line = false;
    for(int i = 0; i < xcom.symbol_table.size(); ++i)
    {
        symbol& s = xcom.symbol_table[i];
        if(s.scope == scope && s.type == type)
        {
            fprintf(output_file, "\t");
            if(scope != SCOPE_GLOBAL)
            {
                fprintf(output_file, "\t");
            }

            if(s.type == SYMBOL_TYPE_PARAM)
            {
                fprintf(output_file, "Param %s", s.name.c_str());
            }

            if(s.type == SYMBOL_TYPE_VAR)
            {
                fprintf(output_file, "Var %s", s.name.c_str());

                if(s.size > 1)
                {
                    fprintf(output_file, " [%d]", s.size);
                }
            }
            fprintf(output_file, "\n");
            is_add_new_line = true;
        }
    }

    if(is_add_new_line)
    {
        fprintf(output_file, "\n");
    }  
}

void code_emit::emit_func(function* f) 
{
    fprintf(output_file, "\tFunc %s\n", f->name.c_str());
    fprintf(output_file, "\t{\n");

    emit_scope_symbol(f->index, SYMBOL_TYPE_PARAM);
    emit_scope_symbol(f->index, SYMBOL_TYPE_VAR);

    if(f->i_code_stream.size() > 0)
    {
        bool is_first_source_line = true;
        for(int i = 0; i < f->i_code_stream.size(); ++i)
        {
            i_code* icode = xcom.xicode.get_icode_by_index(f->index, i);
            switch(icode->type)
            {
            case ICODE_NODE_SOURCE_LINE:
            {
                int last_char_index = icode->source_line.size() - 1;
                if(icode->source_line[last_char_index] == '\n')
                {
                    icode->source_line[last_char_index] = '\0';
                }
                if(!is_first_source_line)
                {
                    fprintf(output_file, "\n");
                }
                fprintf(output_file, "\t\t; %s\n\n", icode->source_line.c_str());
                break;
            }
            case ICODE_NODE_INSTR:
            {
                fprintf(output_file, "\t\t%s", mnemonics_table[icode->instruction.opcode]);
                int opcount = icode->instruction.operands.size();

                if(opcount)
                {
                    fprintf(output_file, "\t");

                    if(strlen(mnemonics_table[icode->instruction.opcode]) < TAB_STOP_WIDTH)
                    {
                        fprintf(output_file, "\t");
                    }
                }

                for(int m = 0; m < opcount; ++m)
                {
                    operand* op = xcom.xicode.get_icode_operand_by_index(icode, m);
                    switch(op->type)
                    {
                    case OP_TYPE_INT:
                        fprintf(output_file, "%d", op->int_literal);
                        break;
                    case OP_TYPE_FLOAT:
                        fprintf(output_file, "%f", op->float_literal);
                        break;
                    case OP_TYPE_STRING_INDEX:
                        fprintf(output_file, "\"%s\"", xcom.get_string_by_index(op->string_index).c_str());
                        break;
                    case OP_TYPE_VAR:
                        fprintf(output_file, "%s", xcom.get_symbol_by_index(op->symbol_index)->name.c_str());
                        break;
                    case OP_TYPE_ARRAY_INDEX_ABS:
                        fprintf(output_file, "%s [ %d ]", xcom.get_symbol_by_index(op->symbol_index)->name.c_str(), op->offset);
                        break;
                    case OP_TYPE_ARRAY_INDEX_VAR:
                        fprintf(output_file, "%s [ %s ]", xcom.get_symbol_by_index(op->symbol_index)->name.c_str(), 
                            xcom.get_symbol_by_index(op->offset_symbol)->name.c_str());
                        break;
                    case OP_TYPE_FUNC_INDEX:
                        fprintf(output_file, "%s", xcom.get_function_by_index(op->symbol_index)->name.c_str());
                        break;
                    case OP_TYPE_REG:
                        fprintf(output_file, "_RetVal");
                        break;
                    case OP_TYPE_JUMP_TARGET_INDEX:
                        fprintf(output_file, "_L%d", op->jump_target_index);
                        break;
                    }//switch(op->type)

                    if(m != opcount - 1)
                    {
                        fprintf(output_file, ", ");
                    }
                }
                fprintf(output_file, "\n");
                break;
            }
            case ICODE_NODE_JUMP_TARGET:
            {
                fprintf(output_file, "\t_L%d:\n", icode->jump_target_index);
                break;
            }
            }//switch(icode->type)

            if(is_first_source_line)
            {
                is_first_source_line = false;
            }
        }
    }
    else
    {
        fprintf(output_file, "\t\t; (No code)\n");
    }

    fprintf(output_file, "\t}");   
}

void code_emit::emit_code()
{
    if(!(output_file = fopen(xcom.output_file_name.c_str(), "wb")))
    {
        xcom.exit_on_error("Could not open output file for output");
    }

    emit_header();

    fprintf(output_file, "; ---- Directives -----------------------------------------------------------------------------\n\n");
    emit_directives();

    fprintf(output_file, "; ---- Global Variables -----------------------------------------------------------------------\n\n");
    emit_scope_symbol(SCOPE_GLOBAL, SYMBOL_TYPE_VAR);

    fprintf(output_file, "; ---- Functions ------------------------------------------------------------------------------\n\n");

    function* f;
    function* mainf = NULL;
    for(int i = 0; i < xcom.function_table.size(); ++i)
    {
        f = &(xcom.function_table[i]);
        if(!f->is_host_api)
        {
            if(f->name == MAIN_FUNC_NAME)
            {
                mainf = f;
            }
            else
            {
                emit_func(f);
                fprintf(output_file, "\n\n");
            }
        }
    }
    
    fprintf(output_file, "; ---- Main -----------------------------------------------------------------------------------");
    if(mainf)
    {
        fprintf(output_file, "\n\n");
        emit_func(mainf);
    }

    fclose(output_file);   
}

}//namespace xcomplier
}//namespace xscript
