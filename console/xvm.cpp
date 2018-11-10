#include "xvm.hpp"

namespace xscript {
namespace xvm {

xvm::xvm()
{
}

xvm::~xvm()
{

}

void xvm::xvm_init()
{
    //initialize the script array
    for(int i = 0; i < MAX_THREAD_COUNT; ++i)
    {
        scripts[i].is_active = false;

        scripts[i].is_running = false;
        scripts[i].is_main_function_present = false;
        scripts[i].is_paused = false;
    }

    //initialize the host API
    for(int i = 0; i < MAX_HOST_API_SIZE; ++i)
    {
        host_apis[i].is_active = false;
    }

    //set up the threads
    current_thread = 0;
    current_thread_mode = THREAD_MODE_MULTI;
}

void xvm::xvm_shutdown ()
{
    //unload any scripts that may still be in memory
    for(int i = 0; i < MAX_THREAD_COUNT; ++i)
    {
        xvm_unload_script(i);
    }
}

int xvm::xvm_load_script(const char* script_name, int& script_index, int thread_timeslice)
{
    //find the next free script index
    bool is_free_thread_found = false;
    for(int i = 0; i < MAX_THREAD_COUNT; ++i)
    {
        if(!scripts[i].is_active)
        {
            script_index = i;
            is_free_thread_found = true;
            break;
        }
    }

    //if a thread wasn't found, return an out of threads error
    if(!is_free_thread_found)
    {
        return XS_LOAD_ERROR_OUT_OF_THREADS;
    }

    //open the input file
    FILE* script_file;
    if(!(script_file = fopen(script_name, "rb")))
    {
        return XS_LOAD_ERROR_FILE_IO;
    }

    //----------read the header------------//
    //read the file's ID(4 bytes) and append a null terminator
    char script_file_ID[5];
    fread(script_file_ID, 4, 1, script_file);
    script_file_ID[4] = '\0';

    //check file's ID
    if(strcmp(script_file_ID, XSE_ID_STRING) != 0)
    {
        return XS_LOAD_ERROR_INVALID_XSE;
    }

    //read the script version(2 bytes)
    int major_version = 0;
    int minor_version = 0;
    fread(&major_version, 1, 1, script_file);
    fread(&minor_version, 1, 1, script_file);

    //check version
    if(major_version != MAJOR_VERSION || minor_version != MINOR_VERSION)
    {
        return XS_LOAD_ERROR_UNSUPPORTED_VERS;
    }

    //read the stack size(4 bytes)
    int stack_size = 0;
    fread(&stack_size, 4, 1, script_file);

    //check for a default stack size request
    stack_size = stack_size == 0 ? DEF_STACK_SIZE : stack_size;
    scripts[script_index].stack.size = stack_size;

    //allocate the runtime stack
    scripts[script_index].stack.elements.resize(stack_size);

    //read the global data size(4 bytes)
    fread(&scripts[script_index].global_data_size, 4, 1, script_file);

    //check for presence of _Main() (1byte)
    fread(&scripts[script_index].is_main_function_present, 1, 1, script_file);

    //read _Main()'s function index(4 bytes)
    fread(&scripts[script_index].main_function_index, 4, 1, script_file);

    //read the priority type(1 byte)
    int priority_type = 0;
    fread(&priority_type, 1, 1, script_file);

    //read the user-defined priority(4 bytes)
    fread(&scripts[script_index].timeslice_duration, 4, 1, script_file);

    //override the script-specified priority if necessary
    if(thread_timeslice != XS_THREAD_PRIORITY_USER)
    {
        priority_type = thread_timeslice;
    }

    //if the priority type is not set to user-defined, fill in the appropriate timeslice duration
    switch(priority_type)
    {
    case XS_THREAD_PRIORITY_LOW:
        scripts[script_index].timeslice_duration = THREAD_PRIORITY_DUR_LOW;
        break;
    case XS_THREAD_PRIORITY_MED:
        scripts[script_index].timeslice_duration = THREAD_PRIORITY_DUR_MED;
        break;
    case XS_THREAD_PRIORITY_HIGH:
        scripts[script_index].timeslice_duration = THREAD_PRIORITY_DUR_HIGH;
        break;
    }

    //--------------read the instruction stream--------------------//
    //read the instruction count (4 bytes)
    int code_stream_size = 0;
    fread(&code_stream_size, 4, 1, script_file);

    //allocate the stream
    scripts[script_index].code_stream.codes.resize(code_stream_size);

    //read the instruction data
    for(int i = 0; i < scripts[script_index].code_stream.codes.size(); ++i)
    {
        //read the opcode(2 bytes)
        scripts[script_index].code_stream.codes[i].opcode = 0;
        fread(&scripts[script_index].code_stream.codes[i].opcode, 2, 1, script_file);
        int oc = scripts[script_index].code_stream.codes[i].opcode;

        //read the operand count(1 byte)
        scripts[script_index].code_stream.codes[i].opcount = 0;
        fread(&scripts[script_index].code_stream.codes[i].opcount, 1, 1, script_file);

        int opcount = scripts[script_index].code_stream.codes[i].opcount;

        //assign the operand list pointer to the instruction stream
        scripts[script_index].code_stream.codes[i].oplist.resize(opcount);
        value_vector& oplist =scripts[script_index].code_stream.codes[i].oplist;

        //read in the operand list(N bytes)
        for(int j = 0; j < opcount; ++j)
        {
            //read in the operand type(1 byte)
            oplist[j].type = 0;
            fread(&oplist[j].type, 1, 1, script_file);

            //depending on the type, read in the operand data
            switch(oplist[j].type)
            {
            case OP_TYPE_INT:
                fread(&oplist[j].int_literal, sizeof(int), 1, script_file);
                break;
            case OP_TYPE_FLOAT:
                fread(&oplist[j].float_literal, sizeof(float), 1, script_file);
                break;
            case OP_TYPE_STRING_INDEX:
                fread(&oplist[j].string_index, sizeof(int), 1, script_file);
                //oplist[j].type = OP_TYPE_STRING_INDEX;
                break;
            case OP_TYPE_INSTR_INDEX:
                fread(&oplist[j].instruction_index, sizeof(int), 1, script_file);
                break;
            case OP_TYPE_ABS_STACK_INDEX:
                fread(&oplist[j].stack_index, sizeof(int), 1, script_file);
                break;
            case OP_TYPE_REL_STACK_INDEX:
                fread(&oplist[j].stack_index, sizeof(int), 1, script_file);
                fread(&oplist[j].offset_index, sizeof(int), 1, script_file);
                break;
            case OP_TYPE_FUNC_INDEX:
                fread(&oplist[j].function_index, sizeof(int), 1, script_file);
                break;
            case OP_TYPE_HOST_API_CALL_INDEX:
                fread(&oplist[j].host_api_index, sizeof(int), 1, script_file);
                break;
            case OP_TYPE_REG:
                fread(&oplist[j].reg, sizeof(int), 1, script_file);
                break;
            }
        }
    }

    //-------------read the string table----------------//
    //read the table size(4 bytes)
    int string_table_size = 0;
    fread(&string_table_size, 4, 1, script_file);

    //if the string table exists, read it
    if(string_table_size > 0)
    {
        scripts[script_index].string_table.resize(string_table_size);

        //read in each string
        for(int i = 0; i < string_table_size; ++i)
        {
            //read in the string size(4 bytes)
            int string_size = 0;
            fread(&string_size, 4, 1, script_file);

            //read in the string data(N bytes) and append the null terminator            
            char ss[string_size + 1];
            fread(ss, string_size, 1, script_file);
            ss[string_size] = '\0';

            //assign the string pointer to the string table
            scripts[script_index].string_table[i] = ss;
        }
    }

    //--------------read the function table--------------//
    int function_table_size = 0;
    fread(&function_table_size, 4, 1, script_file);
    scripts[script_index].function_table.resize(function_table_size);

    //read each function
    for(int i = 0; i < function_table_size; ++ i)
    {
        //read the entry point(4 bytes)
        int entry_point = 0;
        fread(&entry_point, 4, 1, script_file);

        //read the parameter count(1 byte)
        int param_count = 0;
        fread(&param_count, 1, 1, script_file);

        //read the local data size(4 bytes)
        int local_data_size = 0;
        fread(&local_data_size, 4, 1, script_file);

        //calculate the stack size
        int stack_frame_size = param_count + 1 + local_data_size;

        //read the function name length(1 byte)
        int function_name_len = 0;
        fread(&function_name_len, 1, 1, script_file);

        //read the function name (N bytes) and append a null-terminator
        char function_name[function_name_len + 1];
        fread(function_name, function_name_len, 1, script_file);
        function_name[function_name_len] = '\0';
printf("FUNCTION [%d:%s]\n", i, function_name);
        //write everything to the function table
        scripts[script_index].function_table[i].name = function_name;
        scripts[script_index].function_table[i].entry_point = entry_point;
        scripts[script_index].function_table[i].param_count = param_count;
        scripts[script_index].function_table[i].local_data_size = local_data_size;
        scripts[script_index].function_table[i].stack_frame_size = stack_frame_size;
    }

    //-----------read the host api table-------------//
    //read the host api count
    int host_api_table_size = 0;
    fread(&host_api_table_size, 4, 1, script_file);

    //allocate the table
    scripts[script_index].host_api_table.resize(host_api_table_size);

    //read each host API
    for(int i = 0; i < host_api_table_size; ++ i)
    {
        //read the host API call string size(1 byte)
        int host_api_name_len = 0;
        fread(&host_api_name_len, 1, 1, script_file);

        //allocate space for the string
        char host_api_name[host_api_name_len + 1];

        //read the host api
        fread(host_api_name, host_api_name_len, 1, script_file);
        host_api_name[host_api_name_len] = '\0';

        //set host API
        scripts[script_index].host_api_table[i] = host_api_name;
    }

    //------------close the input file----------------//
    fclose(script_file);

    //the script is fully loaded and ready to go, so set the active flag
    scripts[script_index].is_active = true;
    bool ita = is_thread_active(script_index);

    //reset the scritp
    xvm_reset_script(script_index);

    return XS_LOAD_OK;
}

void xvm::xvm_unload_script(int script_index)
{
    if(!scripts[script_index].is_active)
    {
        return;
    }

    /*for(int i = 0; i < scripts[script_index].code_stream.codes.size(); ++i)
    {
        int opcount = scripts[script_index].code_stream.codes[i].opcount;
        value_vector& oplist = scripts[script_index].code_stream.codes[i].oplist;

        for(int j = 0; j < opcount; ++j)
        {
            if(oplist[j].string_literal)
            {
                free(oplist[j].string_literal);
            }
        }
    }

    for(int i = 0; i < scripts[script_index].stack.elements.size(); ++i)
    {
        if(scripts[script_index].stack.elements[i].type == OP_TYPE_STRING_INDEX)
        {
            free(scripts[script_index].stack.elements[i].string_literal);
        }
    }*/

    scripts[script_index].code_stream.codes.clear();
    scripts[script_index].stack.elements.clear();
    scripts[script_index].function_table.clear();
    scripts[script_index].host_api_table.clear();
    scripts[script_index].string_table.clear();
}

void xvm::xvm_reset_script(int script_index)
{
    int main_function_index = scripts[script_index].main_function_index;

    if(scripts[script_index].function_table.size() > 0)
    {
        if(scripts[script_index].is_main_function_present)
        {
            scripts[script_index].code_stream.current_code = scripts[script_index].function_table[main_function_index].entry_point;
        }
    }

    scripts[script_index].stack.top = 0;
    scripts[script_index].stack.frame = 0;

    for(int i = 0; i < scripts[script_index].stack.elements.size(); ++i)
    {
        scripts[script_index].stack.elements[i].type = OP_TYPE_NULL;
    }

    scripts[script_index].is_paused = false;

    //allocate space for the globals
    push_frame(script_index, scripts[script_index].global_data_size);

    //if _Main() is present, push its stack frame
    push_frame(script_index, scripts[script_index].function_table[main_function_index].local_data_size + 1);
}

void xvm::xvm_run_script(int timeslice_duration)
{
    //begin a loop that runs until a keypress. the instruction pointer has already been
    //initialized with a prior call to reset_script(), so execution can begin

    int is_exit_execute_loop = false;
    int main_timeslice_start_time = get_current_time();
    int current_time;

    while(true)
    {
        bool is_still_active = false;
        for(int i = 0; i < MAX_THREAD_COUNT; ++i)
        {
            if(scripts[i].is_active && scripts[i].is_running)
            {
                is_still_active = true;
                break;
            }
        }
        if(!is_still_active)
        {
            break;
        }

        current_time = get_current_time();

        //check for a context switch if the threading mode is set for multithreading
        if(current_thread_mode == THREAD_MODE_MULTI)
        {
            if(current_time > current_thread_active_time + scripts[current_thread].timeslice_duration ||
                !scripts[current_thread].is_running)
            {
                while(true)
                {
                    ++current_thread;
                    if(current_thread >= MAX_THREAD_COUNT)
                    {
                        current_thread = 0;
                    }

                    if(scripts[current_thread].is_active && scripts[current_thread].is_running)
                    {
                        break;
                    }
                }
            }
        }

        //is the script currently paused?
        if(scripts[current_thread].is_paused)
        {
            if(current_time >= scripts[current_thread].pause_end_time)
            {
                scripts[current_thread].is_paused = false;
            }
            else
            {
                continue;
            }
        }

        //make a copy of the instruction pointer to compare later
        int cc = scripts[current_thread].code_stream.current_code;
        assert(cc < scripts[current_thread].code_stream.codes.size());

        //get the current opcode
        int opcode = scripts[current_thread].code_stream.codes[cc].opcode;

        //execute the current instruction based on its opcode, as long as we aren't currently paused
        switch(opcode)
        {
        //Move
        case INSTR_MOV:

        //Arithmetic Operations
        case INSTR_ADD:
        case INSTR_SUB:
        case INSTR_MUL:
        case INSTR_DIV:
        case INSTR_MOD:
        case INSTR_EXP:

        //Bitwise Operations
        case INSTR_AND:
        case INSTR_OR:
        case INSTR_XOR:
        case INSTR_SHL:
        case INSTR_SHR:
        {
            xvm_value dest = resolve_operand_value(0);
            xvm_value source = resolve_operand_value(1);

            switch(opcode)
            {
            case INSTR_MOV:
                if(resolve_operand_ptr(0) != resolve_operand_ptr(1))
                {
                    dest = source;
                    //copy_value(&dest, source);
                }
                break;
            case INSTR_ADD:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal += resolve_operand_as_int(1);
                }
                else
                {
                    dest.float_literal += resolve_operand_as_float(1);
                }
                break;
            case INSTR_SUB:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal -= resolve_operand_as_int(1);
                }
                else
                {
                    dest.float_literal -= resolve_operand_as_float(1);
                }
                break;
            case INSTR_MUL:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal *= resolve_operand_as_int(1);
                }
                else
                {
                    dest.float_literal *= resolve_operand_as_float(1);
                }
                break;
            case INSTR_DIV:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal /= resolve_operand_as_int(1);
                }
                else
                {
                    dest.float_literal /= resolve_operand_as_float(1);
                }
                break;
            case INSTR_MOD:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal %= resolve_operand_as_int(1);
                }
                break;
            case INSTR_EXP:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal = static_cast<int>(pow(dest.int_literal, resolve_operand_as_int(1)));
                }
                else
                {
                    dest.float_literal = static_cast<float>(pow(dest.float_literal, resolve_operand_as_float(1)));
                }
                break;
            case INSTR_AND:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal &= resolve_operand_as_int(1);
                }
                break;
            case INSTR_OR:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal |= resolve_operand_as_int(1);
                }
                break;
            case INSTR_XOR:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal ^= resolve_operand_as_int(1);
                }
                break;
            case INSTR_SHL:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal <<= resolve_operand_as_int(1);
                }
                break;
            case INSTR_SHR:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal >>= resolve_operand_as_int(1);
                }
                break;
            }//switch(opcode)
            *resolve_operand_ptr(0) = dest;
            break;
        }
        //unary operations
        case INSTR_NEG:
        case INSTR_NOT:
        case INSTR_INC:
        case INSTR_DEC:
        {
            xvm_value dest = resolve_operand_value(0);

            switch(opcode)
            {
            case INSTR_NEG:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal = -dest.int_literal;
                }
                else
                {
                    dest.float_literal = -dest.float_literal;
                }
                break;
            case INSTR_NOT:
                if(dest.type == OP_TYPE_INT)
                {
                    dest.int_literal = ~dest.int_literal;
                }
            case INSTR_INC:
                if(dest.type == OP_TYPE_INT)
                {
                    ++dest.int_literal;
                }
                else
                {
                    ++dest.float_literal;
                }
                break;
            case INSTR_DEC:
                if(dest.type == OP_TYPE_INT)
                {
                    --dest.int_literal;
                }
                else
                {
                    --dest.float_literal;
                }
                break;
            }//switch(opcode)
            *resolve_operand_ptr(0) = dest;
            break;
        }
        case INSTR_CONCAT:
        {
            xvm_value dest = resolve_operand_value(0);
            string source_string = resolve_operand_as_string(1);

            if(dest.type != OP_TYPE_STRING_INDEX)
            {
                break;
            }

            const string& old_string = scripts[current_thread].string_table[dest.string_index];
            string new_string = old_string + source_string;
            dest.string_index = add_string_if_new(new_string);

            *resolve_operand_ptr(0) = dest;
            break;
        }
        case INSTR_GETCHAR:
        {
            xvm_value dest = resolve_operand_value(0);
            string source_string = resolve_operand_as_string(1);
            int source_index = resolve_operand_as_int(2);

            char ch[2];
            ch[0] = source_string[source_index];
            ch[1] = '\0';
            dest.string_index = add_string_if_new(ch);

            *resolve_operand_ptr(0) = dest;
            break;
        }
        case INSTR_SETCHAR:
        {
            if(resolve_operand_type(0) != OP_TYPE_STRING_INDEX)
            {
                break;
            }

            int dest_index = resolve_operand_as_int(1);
            string source_string = resolve_operand_as_string(2);
            scripts[current_thread].string_table[resolve_operand_ptr(0)->string_index][dest_index] = source_string[0];
            break;
        }
        case INSTR_JMP:
        {
            int target_index = resolve_operand_as_instruction_index(0);
            scripts[current_thread].code_stream.current_code = target_index;
            break;
        }
        case INSTR_JE:
        case INSTR_JNE:
        case INSTR_JG:
        case INSTR_JL:
        case INSTR_JGE:
        case INSTR_JLE:
        {
            xvm_value v0 = resolve_operand_value(0);
            xvm_value v1 = resolve_operand_value(1);
            int target_index = resolve_operand_as_instruction_index(2);
            bool is_jump = false;

            switch(opcode)
            {
            case INSTR_JE:
                switch(v0.type)
                {
                case OP_TYPE_INT:
                    is_jump = v0.int_literal == v1.int_literal ? true : false;
                    break;
                case OP_TYPE_FLOAT:
                    is_jump = v0.float_literal == v1.float_literal ? true : false;
                    break;
                case OP_TYPE_STRING_INDEX:
                {
                    string& s0 = scripts[current_thread].string_table[v0.string_index];
                    string& s1 = scripts[current_thread].string_table[v1.string_index];
                    is_jump = s0 == s1 ? true : false;
                    break;
                }
                }
                break;
            case INSTR_JNE:
                switch(v0.type)
                {
                case OP_TYPE_INT:
                    is_jump = v0.int_literal != v1.int_literal ? true : false;
                    break;
                case OP_TYPE_FLOAT:
                    is_jump = v0.float_literal != v1.float_literal ? true : false;
                    break;
                case OP_TYPE_STRING_INDEX:
                {
                    string& s0 = scripts[current_thread].string_table[v0.string_index];
                    string& s1 = scripts[current_thread].string_table[v1.string_index];
                    is_jump = s0 == s1 ? true : false;
                    break;
                }
                }
                break;
            case INSTR_JG:
                switch(v0.type)
                {
                case OP_TYPE_INT:
                    is_jump = v0.int_literal > v1.int_literal ? true : false;
                    break;
                case OP_TYPE_FLOAT:
                    is_jump = v0.float_literal > v1.float_literal ? true : false;
                    break;
                }
                break;
            case INSTR_JL:
                switch(v0.type)
                {
                case OP_TYPE_INT:
                    is_jump = v0.int_literal < v1.int_literal ? true : false;
                    break;
                case OP_TYPE_FLOAT:
                    is_jump = v0.float_literal < v1.float_literal ? true : false;
                    break;
                }
                break;
            case INSTR_JGE:
                switch(v0.type)
                {
                case OP_TYPE_INT:
                    is_jump = v0.int_literal >= v1.int_literal ? true : false;
                    break;
                case OP_TYPE_FLOAT:
                    is_jump = v0.float_literal >= v1.float_literal ? true : false;
                    break;
                }
                break;
            case INSTR_JLE:
                switch(v0.type)
                {
                case OP_TYPE_INT:
                    is_jump = v0.int_literal <= v1.int_literal ? true : false;
                    break;
                case OP_TYPE_FLOAT:
                    is_jump = v0.float_literal <= v1.float_literal ? true : false;
                    break;
                }
                break;
            }//opcode

            if(is_jump)
            {
                scripts[current_thread].code_stream.current_code = target_index;
            }

            break;
        }
        case INSTR_PUSH:
        {
            xvm_value v = resolve_operand_value(0);
            push(current_thread, v);
            break;
        }
        case INSTR_POP:
        {
            *resolve_operand_ptr(0) = pop(current_thread);
            break;
        }
        case INSTR_CALL:
        {
            int function_index = resolve_operand_as_function_index(0);

            //advance the instruction pointer so it points to the instruction immediately following the call
            ++scripts[current_thread].code_stream.current_code;
            call_function(current_thread, function_index);
            break;
        }
        case INSTR_RET:
        {
            scripts[current_thread].stack.top = scripts[current_thread].stack.frame;
            xvm_value current_function_data = pop(current_thread);

            //check for the presence of a stack base marker
            if(current_function_data.type ==OP_TYPE_STACK_BASE_MARKER)
            {
                is_exit_execute_loop = true;
            }

            //get the previous function index
            assert(current_function_data.function_index != -1);
            assert(current_function_data.function_index < scripts[current_thread].function_table.size());
            function f = get_function(current_thread, current_function_data.function_index);
            int frame = current_function_data.offset_index;

            //read the return address structure from the stack, which is stored one index below the local data
            xvm_value return_address = get_stack_value(current_thread, scripts[current_thread].stack.top - (f.local_data_size + 1));

            //pop the stack frame along with the return address
            pop_frame(f.stack_frame_size);

            //restore the previous frame index
            scripts[current_thread].stack.frame = frame;

            //make the jump to the return address
            scripts[current_thread].code_stream.current_code = return_address.instruction_index;
            break;
        }
        case INSTR_CALLHOST:
        {
            xvm_value host_api_call = resolve_operand_value(0);
            int host_api_index = host_api_call.host_api_index;
            string host_api_name = get_host_api(host_api_index);
            for(int i = 0; i < MAX_HOST_API_SIZE; ++i)
            {
                if(host_api_name == host_apis[i].name)
                {
                    int thread_index = host_apis[i].thread_index;
                    if(thread_index == current_thread || thread_index == XS_GLOBAL_FUNC)
                    {
                        host_apis[i].function(current_thread);
                        break;
                    }
                }
            }
            break;
        }
        case INSTR_PAUSE:
        {
            int pause_duration = resolve_operand_as_int(0);
            scripts[current_thread].pause_end_time = current_time + pause_duration;
            scripts[current_thread].is_paused = true;
            break;
        }
        case INSTR_EXIT:
        {
            xvm_value exit_code = resolve_operand_value(0);
            int ec = exit_code.int_literal;
            scripts[current_thread].is_running = false;
            break;
        }
        }//switch(opcode)

        if(cc == scripts[current_thread].code_stream.current_code)
        {
            ++scripts[current_thread].code_stream.current_code;
        }

        if(timeslice_duration != XS_INFINITE_TIMESLICE)
        {
            if(current_time > main_timeslice_start_time + timeslice_duration)
            {
                break;
            }
        }

        if(is_exit_execute_loop)
        {
            break;
        }
    }
}

void xvm::xvm_start_script(int script_index)
{
    if(!is_thread_active(script_index))
    {
        return;
    }

    scripts[script_index].is_running = true;

    current_thread = script_index;
    current_thread_active_time = get_current_time();
}

void xvm::xvm_stop_script(int script_index)
{
    if(!is_thread_active(script_index))
    {
        return;
    }

    scripts[script_index].is_running = false;  
}

void xvm::xvm_pause_script(int script_index, int duration)
{
    if(!is_thread_active(script_index))
    {
        return;
    }

    scripts[script_index].is_paused = true;
    scripts[script_index].pause_end_time = get_current_time() + duration;    
}

void xvm::xvm_unpause_script(int script_index)
{
    if(!is_thread_active(script_index))
    {
        return;
    }

    scripts[script_index].is_paused = false;    
}

void xvm::xvm_pass_int_param(int script_index, int v)
{
    xvm_value p;
    p.type = OP_TYPE_INT;
    p.int_literal = v;

    push(script_index, p);
}

void xvm::xvm_pass_float_param(int script_index, float v)
{
    xvm_value p;
    p.type = OP_TYPE_FLOAT;
    p.float_literal = v;

    push(script_index, p);
}

void xvm::xvm_pass_string_param(int script_index, const char* str)
{
    xvm_value p;
    p.type = OP_TYPE_STRING_INDEX;
    p.string_index = add_string_if_new(str);

    push(script_index, p);
}

int xvm::xvm_get_return_as_int(int script_index)
{
    if(!is_thread_active(script_index))
    {
        return 0;
    }

    return scripts[script_index]._RetVal.int_literal; 
}

float xvm::xvm_get_return_as_float(int script_index)
{
    if(!is_thread_active(script_index))
    {
        return 0;
    }

    return scripts[script_index]._RetVal.float_literal; 
}

string xvm::xvm_get_return_as_string(int script_index)
{
    if(!is_thread_active(script_index))
    {
        return NULL;
    }

    return get_string(scripts[script_index]._RetVal.string_index); 
}

void xvm::xvm_call_script_function(int script_index, const char* fname)
{
    if(!is_thread_active(script_index))
    {
        return;
    }

    ///calling the function
    //preserve the current state of the VM
    int prev_thread = current_thread;
    int prev_thread_mode = current_thread_mode;

    //set the threading mode for single-threaded execution
    current_thread_mode = THREAD_MODE_SINGLE;

    //set the active thread to the one specified
    current_thread = script_index;

    //get the function's index based on it's name
    int function_index = get_function_index_by_name(script_index, fname);
    if(function_index == -1)
    {
        return;
    }

    //call the function
    call_function(script_index, function_index);
   
    //set the stack base
    xvm_value stack_base = get_stack_value(current_thread, scripts[current_thread].stack.top - 1);
    stack_base.type = OP_TYPE_STACK_BASE_MARKER;
    set_stack_value(current_thread, scripts[current_thread].stack.top - 1, stack_base);

    //allow the script code to execute uninterrupted until the function returns
    xvm_run_script(XS_INFINITE_TIMESLICE);

    ///handling the function return

    //restore the VM state
    current_thread = prev_thread;
    current_thread_mode = prev_thread_mode;
}

void xvm::xvm_invoke_script_function(int script_index, const char* fname)
{
    if(!is_thread_active(script_index))
    {
        return;
    }

    int function_index = get_function_index_by_name(script_index, fname);
    if(function_index == -1)
    {
        return;
    }    

    call_function(script_index, function_index);
}

void xvm::xvm_register_host_api(int script_index, const char* fname, host_api_function_ptr fn)
{
    for(int i = 0; i < MAX_HOST_API_SIZE; ++i)
    {
        if(!host_apis[i].is_active)
        {
            host_apis[i].thread_index = script_index;
            host_apis[i].function = fn;
            host_apis[i].name = fname;
            string_to_upper(const_cast<char*>(host_apis[i].name.c_str()));
            host_apis[i].is_active = true;

            break;
        }
    }
}

int xvm::xvm_get_param_as_int(int script_index, int param_index)
{
    int top = scripts[script_index].stack.top;
    xvm_value p = scripts[script_index].stack.elements[top - (param_index + 1)];

    return cast_value_to_int(p);
}

float xvm::xvm_get_param_as_float(int script_index, int param_index)
{
    int top = scripts[script_index].stack.top;
    xvm_value p = scripts[script_index].stack.elements[top - (param_index + 1)];

    return cast_value_to_float(p);
}

string xvm::xvm_get_param_as_string(int script_index, int param_index)
{
    int top = scripts[script_index].stack.top;
    xvm_value p = scripts[script_index].stack.elements[top - (param_index + 1)];

    return cast_value_to_string(p);   
}

void xvm::xvm_return_from_host(int script_index, int param_count)
{
    scripts[script_index].stack.top -= param_count;
}

void xvm::xvm_return_int_from_host(int script_index, int param_count, int v)
{
    scripts[script_index].stack.top -= param_count;

    scripts[script_index]._RetVal.type = OP_TYPE_INT;
    scripts[script_index]._RetVal.int_literal = v;
}

void xvm::xvm_return_float_from_host(int script_index, int param_count, float v)
{
    scripts[script_index].stack.top -= param_count;

    scripts[script_index]._RetVal.type = OP_TYPE_FLOAT;
    scripts[script_index]._RetVal.float_literal = v;
}

void xvm::xvm_return_string_from_host(int script_index, int param_count, char* str)
{
    scripts[script_index].stack.top -= param_count;

    xvm_value return_value;
    return_value.type = OP_TYPE_STRING_INDEX;
    return_value.string_index = add_string_if_new(str);

    scripts[script_index]._RetVal = return_value;
    //copy_value(&scripts[script_index]._RetVal, return_value);
}

int xvm::cast_value_to_int(const xvm_value& v)
{
    switch(v.type)
    {
    case OP_TYPE_INT:
        return v.int_literal;
    case OP_TYPE_FLOAT:
        return static_cast<int>(v.float_literal);
    case OP_TYPE_STRING_INDEX:
        return atoi(get_string(v.string_index).c_str());
    default:
        return 0;
    }
}

float xvm::cast_value_to_float(const xvm_value& v)
{
    switch(v.type)
    {
    case OP_TYPE_INT:
        return static_cast<float>(v.int_literal);
    case OP_TYPE_FLOAT:
        return v.float_literal;
    case OP_TYPE_STRING_INDEX:
        return atof(get_string(v.string_index).c_str());
    default:
        return 0;
    }    
}

string xvm::cast_value_to_string(const xvm_value& v)
{
    char s[MAX_COERCION_STRING_SIZE];

    switch(v.type)
    {
    case OP_TYPE_INT:
        sprintf(s, "%d", v.int_literal);
        return s;
    case OP_TYPE_FLOAT:
        sprintf(s, "%f", v.float_literal);
        return s;
    case OP_TYPE_STRING_INDEX:
        return get_string(v.string_index);
    default:
        return "";
    }   
}

#define ASSERT_OPERAND_INDEX(index) assert((index) < scripts[current_thread].code_stream.codes[scripts[current_thread].code_stream.current_code].oplist.size())
int xvm::get_operand_type(int index)
{
    ASSERT_OPERAND_INDEX(index);

    int cc = scripts[current_thread].code_stream.current_code;
    return scripts[current_thread].code_stream.codes[cc].oplist[index].type;
}

int xvm::resolve_operand_stack_index(int index)
{
    ASSERT_OPERAND_INDEX(index);

    int cc = scripts[current_thread].code_stream.current_code;
    xvm_value& op = scripts[current_thread].code_stream.codes[cc].oplist[index];

    switch(op.type)
    {
    case OP_TYPE_ABS_STACK_INDEX:
        return op.stack_index;
    case OP_TYPE_REL_STACK_INDEX:
    {
        int base_index = op.stack_index;
        int offset_index = op.offset_index;

        xvm_value v = get_stack_value(current_thread, offset_index);

        return base_index + v.int_literal;
    }
    default:
        return 0;
    }   
}

xvm_value xvm::resolve_operand_value(int index)
{
    ASSERT_OPERAND_INDEX(index);

    int cc = scripts[current_thread].code_stream.current_code;
    xvm_value& op = scripts[current_thread].code_stream.codes[cc].oplist[index];

    switch(op.type)
    {
    case OP_TYPE_ABS_STACK_INDEX:
    case OP_TYPE_REL_STACK_INDEX:
    {
        int abs_index = resolve_operand_stack_index(index);
        return get_stack_value(current_thread, abs_index);
    }
    case OP_TYPE_REG:
        return scripts[current_thread]._RetVal;
    default:
        return op;
    }  
}

int xvm::resolve_operand_type(int index)
{
    xvm_value v = resolve_operand_value(index);

    return v.type;
}

int xvm::resolve_operand_as_int(int index)
{
    xvm_value v = resolve_operand_value(index);

    return cast_value_to_int(v);   
}

float xvm::resolve_operand_as_float(int index)
{
    xvm_value v = resolve_operand_value(index);

    return cast_value_to_float(v); 
}

string xvm::resolve_operand_as_string(int index)
{
    xvm_value v = resolve_operand_value(index);

    return cast_value_to_string(v); 
}

int xvm::resolve_operand_as_instruction_index(int index)
{
    xvm_value v = resolve_operand_value(index);

    return v.instruction_index;
}

int xvm::resolve_operand_as_function_index(int index)
{
    xvm_value v = resolve_operand_value(index);

    return v.function_index;
}

string xvm::resolve_operand_as_host_api(int index)
{
    xvm_value v = resolve_operand_value(index);

    return get_host_api(v.host_api_index);
}

xvm_value* xvm::resolve_operand_ptr(int index)
{
    int t = get_operand_type(index);

    switch(t)
    {
    case OP_TYPE_ABS_STACK_INDEX:
    case OP_TYPE_REL_STACK_INDEX: 
    {  
        int stack_index = resolve_operand_stack_index(index);
        return &scripts[current_thread].stack.elements[resolve_stack_index(stack_index)];
    }
    case OP_TYPE_REG:
        return &scripts[current_thread]._RetVal;
    }

    return NULL;
}

xvm_value xvm::get_stack_value(int script_index, int index)
{
    return scripts[script_index].stack.elements[resolve_stack_index(index)];
}

void xvm::set_stack_value(int script_index, int index, const xvm_value& v)
{
    scripts[script_index].stack.elements[resolve_stack_index(index)] = v;
}

void xvm::push(int script_index, const xvm_value& v)
{
    int top = scripts[script_index].stack.top;

    scripts[script_index].stack.elements[top] = v;
    //copy_value(&scripts[script_index].stack.elements[top], v);

    ++scripts[script_index].stack.top;
}

xvm_value xvm::pop(int script_index)
{
    --scripts[script_index].stack.top;

    int top = scripts[script_index].stack.top;

    xvm_value v = scripts[script_index].stack.elements[top];
    //copy_value(&v, scripts[script_index].stack.elements[top]);
    return v;
}

void xvm::push_frame(int script_index, int size)
{
    scripts[script_index].stack.top += size;
    scripts[script_index].stack.frame = scripts[script_index].stack.top;
}

void xvm::pop_frame(int size)
{
    scripts[current_thread].stack.top -= size;
}

int xvm::get_function_index_by_name(int script_index, const char* str)
{
    string fname = str;
    string_to_upper(const_cast<char*>(fname.c_str()));

    for(int i = 0; i < scripts[script_index].function_table.size(); ++i)
    {
        if(fname == scripts[script_index].function_table[i].name)
        {
            return i;
        }
    }

    return -1;
}

function xvm::get_function(int script_index, int index)
{
    return scripts[script_index].function_table[index];
}

string xvm::get_host_api(int index)
{
    return scripts[current_thread].host_api_table[index];
}

int xvm::get_current_time()
{
    return get_tick_count();
}

void xvm::call_function(int script_index, int index)
{
    function f = get_function(script_index, index);

    //save the current stack frame index
    int frame = scripts[script_index].stack.frame;

    //push the return address, which is the current instruction
    xvm_value return_address;
    return_address.instruction_index = scripts[script_index].code_stream.current_code;
    push(script_index, return_address);

    //push the stack frame +1(the extra space is for the function index we'll put on the stack after it)
    push_frame(script_index, f.local_data_size + 1);

    //write the function index and old stack frame to the top of the stack
    xvm_value current_function_data;
    current_function_data.function_index = index;
    current_function_data.offset_index = frame;
    set_stack_value(script_index, scripts[script_index].stack.top - 1, current_function_data);

    //let the caller make the jump to the entry point
    scripts[script_index].code_stream.current_code = f.entry_point;
}

int xvm::add_string_if_new(const string& str)
{
    for(int i = 0; i < scripts[current_thread].string_table.size(); ++i)
    {
        if(scripts[current_thread].string_table[i] == str)
        {
            return i;
        }
    }

    scripts[current_thread].string_table.push_back(str);
    return scripts[current_thread].string_table.size() - 1;
}

string xvm::get_string(int sindex)
{
    if(sindex >= scripts[current_thread].string_table.size())
    {
        return "";
    }

    return scripts[current_thread].string_table[sindex];
}
    
}//namespace xvm
}//namespace xscript
