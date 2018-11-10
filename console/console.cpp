#include <stdio.h>
#include "xvm.hpp"
#include "../common/utility.hpp"

xscript::xvm::xvm vm;

void host_api_print(int script_index)
{
    string s = vm.xvm_get_param_as_string(script_index, 0);
    printf("%s", s.c_str());
    vm.xvm_return_from_host(script_index, 0);
}

void host_api_print_newline(int script_index)
{
    printf("\n");
    vm.xvm_return_from_host(script_index, 0);
}

void host_api_print_tab(int script_index)
{
    printf("\t");
    vm.xvm_return_from_host(script_index, 0);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage:\tx script.XSE\n");
        return 0;
    }

    printf("XVM:1.0\n");
    printf("XScript Virtual Machine\n");
    printf("\n");

    vm.xvm_init();

    int script_index;
    int error_code = vm.xvm_load_script(argv[1], script_index, XS_THREAD_PRIORITY_USER);

    if(error_code != XS_LOAD_OK)
    {
        printf("ERROR: ");
        switch(error_code)
        {
        case XS_LOAD_ERROR_FILE_IO:
            printf("File I/O error");
            break;
        case XS_LOAD_ERROR_INVALID_XSE:
            printf("Invalid .XSE file");
            break;
        case XS_LOAD_ERROR_UNSUPPORTED_VERS:
            printf("Unsupported .XSE version");
            break;
        case XS_LOAD_ERROR_OUT_OF_MEMORY:
            printf("Out of memory");
            break;
        case XS_LOAD_ERROR_OUT_OF_THREADS:
            printf("Out of threads");
            break;
        }
        printf("\n");

        return 0;
    }
    else
    {
        printf("Script loaded successfully.\n");
    }
    printf("\n");

    vm.xvm_register_host_api(XS_GLOBAL_FUNC, "PrintString", host_api_print);
    vm.xvm_register_host_api(XS_GLOBAL_FUNC, "PrintNewline", host_api_print_newline);
    vm.xvm_register_host_api(XS_GLOBAL_FUNC, "PrintTab", host_api_print_tab);

    vm.xvm_start_script(script_index);

    while(!kbhit())
    {
        //printf("entry script from C++\n");
        vm.xvm_run_script(500);
    }

    vm.xvm_shutdown();
    printf("XVM shutdown !!!\n\n\n");
    return 0;
}
