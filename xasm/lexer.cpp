#include "lexer.hpp"
#include "utility.hpp"
#include "error_code.hpp"

namespace xscript {
namespace xasm {

lexer::lexer(instruction_set& inn)
:   iset(inn)
{
}

lexer::~lexer()
{
}

void lexer::load_source_file(const string& source_file_name)
{
    FILE* source_file;

    if(!(source_file = fopen(source_file_name.c_str(), "r")))
    {
        exit_on_error("Could not open source file");
    }

    char line[MAX_SOURCE_LINE_SIZE];
    while(!feof(source_file))
    {
        memset(line, 0, MAX_SOURCE_LINE_SIZE);
        fgets(line, MAX_SOURCE_LINE_SIZE, source_file);

        strip_comments(line);
        trim_whitespace(line);

        int ssize = strlen(line);
        int llen = ssize - 1;
        if(line[llen] != '\n')
        {
            line[llen + 1] = '\n';
            //line[llen + 2] = '\0';
            ssize += 1;
        }

        string sline(line, ssize);
        source_codes.push_back(sline);
    }

    fclose(source_file);
}

void lexer::reset_lexer()
{
    current_source_line = 0;
    index_start = 0;
    index_end = 0;

    current_token = TOKEN_TYPE_INVALID;
    current_lex_state = LEX_STATE_START;
}

char lexer::get_next_char()
{
    return source_codes[current_source_line][index_end++];
}

token lexer::read_next_token()
{
    index_start = index_end;
    if(index_start >= source_codes[current_source_line].size())
    {
        if(!skip_to_next_line())
        {
            return END_OF_TOKEN_STREAM;
        }
    }

    current_lex_state = LEX_STATE_START;
    bool lexeme_done = false;
    char cc;
    int next_lexeme_char_index = 0;
    bool add_current_char;

    while(true)
    {
        cc = get_next_char();
        if(cc == '\0')
        {
            break;
        }

        add_current_char = true;
        switch(current_lex_state)
        {
        case LEX_STATE_START:
            if(cc == '"')
            {
                int xxxx = 0;
            }
            if(is_char_whitespace(cc))
            {
                ++index_start;
                add_current_char = false;
            }
            else if(is_char_numeric(cc))
            {
                current_lex_state = LEX_STATE_INT;
            }
            else if(cc == '.')
            {
                current_lex_state = LEX_STATE_FLOAT;
            }
            else if(cc == '"')
            {
                add_current_char = false;
                current_lex_state = LEX_STATE_STRING;
            }
            else if(is_char_identifier(cc))
            {
                current_lex_state = LEX_STATE_IDENT;
            }
            else if(is_char_delimiter(cc))
            {
                current_lex_state = LEX_STATE_DELIM;
            }            
            else
            {
                exit_on_code_error(ERROR_MSSG_INVALID_INPUT);
            }
            break;
        case LEX_STATE_INT:
            if(!is_char_numeric(cc))
            {
                if(cc == '.')
                {
                    current_lex_state = LEX_STATE_FLOAT;
                }
                else if(is_char_whitespace(cc) || is_char_delimiter(cc))
                {
                    add_current_char = false;
                    lexeme_done = true;
                }
                else
                {
                    exit_on_code_error(ERROR_MSSG_INVALID_INPUT);
                }                
            }
            break;
        case LEX_STATE_FLOAT:
            if(!is_char_numeric(cc))
            {
                if(is_char_whitespace(cc) || is_char_delimiter(cc))
                {
                    add_current_char = false;
                    lexeme_done = true;
                }
                else
                {
                    exit_on_code_error(ERROR_MSSG_INVALID_INPUT);
                }                
            }
            break;
        case LEX_STATE_IDENT:
            if(!is_char_identifier(cc))
            {
                if(is_char_whitespace(cc) || is_char_delimiter(cc))
                {
                    add_current_char = false;
                    lexeme_done = true;
                }
                else
                {
                    exit_on_code_error(ERROR_MSSG_INVALID_INPUT);
                }                
            }
            break;
        case LEX_STATE_DELIM:
            add_current_char = false;
            lexeme_done = true;
            break;
        case LEX_STATE_STRING:
            if(cc == '"')
            {
                add_current_char = false;
                current_lex_state = LEX_STATE_STRING_CLOSE_QUOTE;
            }
            else if(cc == '\\')
            {
                add_current_char = false;
                current_lex_state = LEX_STATE_STRING_ESCAPE;
            }
            break;
        case LEX_STATE_STRING_ESCAPE:
            current_lex_state = LEX_STATE_STRING;
            break;
        case LEX_STATE_STRING_CLOSE_QUOTE:
            add_current_char = false;
            lexeme_done = true;
            break;
        }//switch(current_lex_state)

        if(add_current_char)
        {
            current_lexeme[next_lexeme_char_index] = cc;
            ++next_lexeme_char_index;
        }

        if(lexeme_done)
        {
            break;
        }
    }
    current_lexeme[next_lexeme_char_index] = '\0';
    if(current_lex_state != LEX_STATE_STRING_CLOSE_QUOTE)
    {
        strupr(current_lexeme);
    }

    //retract the lexeme end index by one
    --index_end;

    //determine the token type
    token token_type;
    switch(current_lex_state)
    {
    case LEX_STATE_INT:
        token_type = TOKEN_TYPE_INT;
        break;
    case LEX_STATE_FLOAT:
        token_type = TOKEN_TYPE_FLOAT;
        break;
    case LEX_STATE_IDENT:
        token_type = TOKEN_TYPE_IDENT;

        if(iset.is_instruction(current_lexeme))
        {
            token_type = TOKEN_TYPE_INSTR;
        }
        if(strcmp(current_lexeme, "SETSTACKSIZE") == 0)
        {
            token_type = TOKEN_TYPE_SETSTACKSIZE;
        }
        if(strcmp(current_lexeme, "SETPRIORITY") == 0)
        {
            token_type = TOKEN_TYPE_SETPRIORITY;
        }
        if(strcmp(current_lexeme, "VAR") == 0)
        {
            token_type = TOKEN_TYPE_VAR;
        }
        if(strcmp(current_lexeme, "FUNC") == 0)
        {
            token_type = TOKEN_TYPE_FUNC;
        }
        if(strcmp(current_lexeme, "PARAM") == 0)
        {
            token_type = TOKEN_TYPE_PARAM;
        }
        if(strcmp(current_lexeme, "_RETVAL") == 0)
        {
            token_type = TOKEN_TYPE_REG_RETVAL;
        }

        break;
    case LEX_STATE_DELIM:
        switch(current_lexeme[0])
        {
        case ';':
            token_type = TOKEN_TYPE_COLON;
            break;
        case '[':
            token_type = TOKEN_TYPE_OPEN_BRACKET;
            break;
        case ']':
            token_type = TOKEN_TYPE_CLOSE_BRACKET;
            break;
        case ',':
            token_type = TOKEN_TYPE_COMMA;
            break;
        case '{':
            token_type = TOKEN_TYPE_OPEN_BRACE;
            break;
        case '}':
            token_type = TOKEN_TYPE_CLOSE_BRACE;
            break;
        case '\n':
            token_type = TOKEN_TYPE_NEWLINE;
            break;
        }
        break;
    case LEX_STATE_STRING_CLOSE_QUOTE:
        token_type = TOKEN_TYPE_STRING;
        break;
    default:
        token_type = END_OF_TOKEN_STREAM;
    }    
    current_token = token_type;
    return token_type;
}

char lexer::read_look_ahead_char()
{
    int cline = current_source_line;
    int index = index_end;

    if(current_lex_state != LEX_STATE_STRING)
    {
        while(true)
        {
            if(index >= source_codes[cline].size())
            {
                cline += 1;
                if(cline >= source_codes.size())
                {
                    return 0;
                }

                index = 0;
            }

            if(!is_char_whitespace(source_codes[cline][index]))
            {
                break;
            }

            ++index;
        }
    }    

    return source_codes[cline][index];
}

bool lexer::skip_to_next_line()
{
    ++current_source_line;

    if(current_source_line >= source_codes.size())
    {
        return false;
    }

    index_start = 0;
    index_end = 0;

    return true;       
}

void lexer::exit_on_error(const char* emsg)
{
    printf("Fatal Error: %s.\n", emsg);

    exit(0);
}

void lexer::exit_on_code_error(const char* emsg)
{
    printf("Error: %s.\n\n", emsg);
    printf("Line %d\n", current_source_line);

    char line[MAX_SOURCE_LINE_SIZE];
    strcpy(line, source_codes[current_source_line].c_str());

    for(int i = 0; i < strlen(line); ++i)
    {
        if(line[i] == '\t')
        {
            line[i] = ' ';
        }
    }

    printf("%s", line);

    for(int i = 0; i < index_start; ++i)
    {
        printf(" ");
    }
    printf("^\n");

    printf("Could not assemble.\n");

    exit(0);
}

void lexer::exit_on_char_expected_error(char c)
{
    string expected_msg = c + " expected";
    exit_on_code_error(expected_msg.c_str());    
}

}//namespace xasm
}//namespace xscript

