#include "utility.hpp"

char* strupr(char* str)
{
    char* ptr = str;
    while(*ptr != '\0')
    {
        if(islower(*ptr))
        {
            *ptr = toupper(*ptr);
        }
        ptr++;
    }

    return str;
}

void strip_comments(char* str)
{
    int slen = strlen(str);
    bool in_string = false;
    for(int i = 0; i < slen - 1; ++i)
    {
        if(str[i] == '"')
        {
            if(in_string)
            {
                in_string = false;
            }
            else
            {
                in_string = true;
            }
        }

        if(str[i] == ';')
        {
            if(!in_string)
            {
                str[i] = '\n';
                str[i + 1] = '\0';
                break;
            }
        }
    }
}

void trim_whitespace(char* str)
{
    unsigned int slen = strlen(str);
    unsigned int pad_len;
    unsigned int current_char_index; 
    
    if(slen > 1)
    {
        for(current_char_index = 0; current_char_index < slen; ++current_char_index)
        {
            if(!is_char_whitespace(str[current_char_index]))
            {
                break;
            }
        }

        pad_len = current_char_index;
        if(pad_len)
        {
            for(current_char_index = pad_len; current_char_index < slen; ++current_char_index)
            {
                str[current_char_index - pad_len] = str[current_char_index];
            }
            for(current_char_index = slen - pad_len; current_char_index < slen; ++current_char_index)
            {
                str[current_char_index] = ' ';
            }
        }

        for(current_char_index = slen - 1; current_char_index > 0; --current_char_index)
        {
            if(!is_char_whitespace(str[current_char_index]))
            {
                str[current_char_index + 1] = '\0';
                break;
            }
        }
    } 
}

bool is_char_whitespace(char c)
{
    if(c == ' ' || c == '\t')
    {
        return true;
    }

    return false;
}

bool is_char_numeric(char c)
{
    if(c >= '0' && c <= '9')
    {
        return true;
    }

    return false;
}

bool is_char_identifier(char c)
{
    if( (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        c == '_')
    {
        return true;
    }

    return false;
}

bool is_char_delimiter(char c)
{
    if( c == ':' || c == ',' || c == '"' ||
        c == '[' || c == ']' ||
        c == '{' || c == '}' ||
        is_char_whitespace(c) || c == '\n')
    {
        return true;
    }

    return false;
}

bool is_string_whitespace(char* str)
{
    if(!str)
    {
        return false;
    } 

    int slen = strlen(str);
    if(slen == 0)
    {
        return true;
    }

    for(int i = 0; i < slen; ++i)
    {
        if(!is_char_whitespace(str[i]) && str[i] != '\n')
        {
            return false;
        }
    }

    return true;
}

bool is_string_indentifier(char* str)
{
    if(!str)
    {
        return false;
    }   

    int slen = strlen(str);
    if(slen == 0)
    {
        return false;
    }

    if(is_char_numeric(str[0]))
    {
        return false;
    }

    for(int i = 0; i < slen; ++i)
    {
        if(!is_char_identifier(str[i]))
        {
            return false;
        }
    }

    return true;
}

bool is_string_integer(char* str)
{
    if(!str)
    {
        return false;
    }

    int slen = strlen(str);
    if(slen == 0)
    {
        return false;
    }

    for(int i = 1; i < slen; ++i)
    {
        if(!is_char_numeric(str[i]))
        {
            return false;
        }
    }

    if(str[0] == '-' && slen < 2)
    {
        return false;
    }

    if(str[0] != '-' && !is_char_numeric(str[0]))
    {
        return false;
    }

    return true;
}

bool is_string_float(char* str)
{
    if(!str)
    {
        return false;
    }

    int slen = strlen(str);
    if(slen == 0)
    {
        return false;
    }

    int point_count = 0;
    int minus_index = 0;
    for(int i = 0; i < slen; ++i)
    {
        if(!is_char_numeric(str[i]) && !(str[i] == '.') && !(str[i] == '-'))
        {
            return false;
        }

        if(str[i] == '.')
        {
            point_count++;
        }
        else if(str[i] == '-')
        {
            minus_index = i;
        }
    }

    if(point_count > 1 || minus_index > 0)
    {
        return false;
    }

    if(point_count != 1)
    {
        return false;
    }

    return true;
}
