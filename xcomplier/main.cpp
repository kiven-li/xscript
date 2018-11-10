#include "xcomplier.hpp"

void print_usage()
{
    printf("Usage:\txscript Source.XSS [Output.XASM] [Options]\n");
    printf("\n");
    printf("\t-S:Size      Sets the stack size (must be decimal integer value)\n");
    printf("\t-P:Priority  Sets the thread priority: Low, Med, High or timeslice\n");
    printf("\t             duration (must be decimal integer value)\n");
    printf("\t-A           Preserve assembly output file\n");
    printf("\t-N           Don't generate .XSE (preserves assembly output file)\n");
    printf("\n");
    printf("Notes:\n");
    printf("\t- File extensions are not required.\n");
    printf("\t- Executable name is optional; source name is used by default.\n");
    printf("\n");    
}

void read_command_line_param(int argc, char* argv[], xscript::xcomplier::xcomplier& xcom)
{
    char option[32];
    char value[32];

    for(int i = 0; i < argc; ++i)
    {
        string_to_upper(argv[i]);

        if(argv[i][0] == '-')
        {
            int cchar_index;
            int option_size;
            char cchar;

            cchar_index = 1;
            while(true)
            {
                cchar = argv[i][cchar_index];
                if(cchar == ':' || cchar == '\0')
                {
                    break;
                }
                else
                {
                    option[cchar_index - 1] = cchar;
                }

                ++cchar_index;
            }
            option[cchar_index - 1] = '\0';

            if(strstr(argv[i], ":"))
            {
                ++cchar_index;
                option_size = cchar_index;

                value[0] = '\0';
                while(true)
                {
                    if(cchar_index > strlen(argv[i]))
                    {
                        break;
                    }
                    else
                    {
                        cchar = argv[i][cchar_index];
                        value[cchar_index - option_size] = cchar;
                    }
                    ++cchar_index;
                }
                value[cchar_index - option_size] = '\0';

                if(!strlen(value))
                {
                    printf("Invalid value for -%s option", option);
                    exit(1);
                }
            }

            if(strcmp(option, "S") == 0)
            {
                xcom.xheader.stack_size = atoi(value);
            }
            else if(strcmp(option, "P") == 0)
            {
                if(strcmp(value, PRIORITY_LOW_KEYWORD) == 0)
                {
                    xcom.xheader.priority_type = PRIORITY_LOW;
                }
                else if(strcmp(value, PRIORITY_MED_KEYWORD) == 0)
                {
                    xcom.xheader.priority_type = PRIORITY_MED;
                }
                else if(strcmp(value, PRIORITY_HIGH_KEYWORD) == 0)
                {
                    xcom.xheader.priority_type = PRIORITY_HIGH;
                }
                else
                {
                    xcom.xheader.priority_type = PRIORITY_USER;
                    xcom.xheader.user_priority = atoi(value);
                }
            }
            else if(strcmp(option, "A") == 0)
            {
                xcom.preserve_output_file = true;
            }
            else if(strcmp(option, "N") == 0)
            {
                xcom.generate_xse = false;
                xcom.preserve_output_file = false;
            }
            else
            {
                printf("Unrecognized option: \"%s\"", option);
                exit(1);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        print_usage();
        return 0;
    }

    //parse source file name & output file name
    string source_file_name = argv[1];
    string output_file_name = source_file_name + OUTPUT_FILE_EXT;

    if(argv[2] && argv[2][0] != '-')
    {
        output_file_name = argv[2];
    }

    xscript::xcomplier::xcomplier xcom;
    read_command_line_param(argc, argv, xcom);

    xcom.init(source_file_name, output_file_name);
    xcom.complie();
    xcom.shut_down();

    return 0;
}
