#include <stdio.h>
#include "xvm.hpp"
#include "../common/utility.hpp"

xscript::xvm::xvm vm;

void host_api_print_string(int script_index)
{
    string s = vm.xvm_get_param_as_string(script_index, 0);
    int count = vm.xvm_get_param_as_int(script_index, 1);

    for(int i = 0; i < count; ++i)
    {
        printf("\t%s\n", s.c_str());
    }

    vm.xvm_return_string_from_host(script_index, 2, "this is a return value.");
}

main()
{
    printf("XVM Final\n");
    printf("XScript Virtual Machine\n");
    printf("\n");

    vm.xvm_init();

    int script_index;
    int error_code = vm.xvm_load_script("script.xse", script_index, XS_THREAD_PRIORITY_USER);

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

    vm.xvm_register_host_api(XS_GLOBAL_FUNC, "PrintString", host_api_print_string);

    vm.xvm_start_script(script_index);

    printf("Calling DoStuff() asynchronously:\n");
    printf("\n");

    vm.xvm_call_script_function(script_index, "DoStuff");

    float pi = vm.xvm_get_return_as_float(script_index);
    printf("\nReturn value received from script PI=%f\n", pi);
    printf("\n");

    printf("Invoking InvokeLoop () (Press any key to stop):\n");
    printf("\n");

    vm.xvm_invoke_script_function(script_index, "InvokeLoop");
    while(!kbhit())
    {
        printf("entry script from C++\n");
        vm.xvm_run_script(500);
    }

    vm.xvm_shutdown();
    printf("XVM shutdown !!!\n\n\n");
    return 0;
}
