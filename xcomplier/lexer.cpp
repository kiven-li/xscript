#include "lexer.hpp"
#include "xcomplier.hpp"

namespace xscript {
namespace xcomplier {

operand_state operand_chars_0[MAX_OP_STATE_COUNT] = { 
    { '+', 0, 2, 0 }, 
    { '-', 2, 2, 1 }, 
    { '*', 4, 1, 2 }, 
    { '/', 5, 1, 3 },
    { '%', 6, 1, 4 }, 
    { '^', 7, 1, 5 }, 
    { '&', 8, 2, 6 }, 
    { '|', 10, 2, 7 },
    { '#', 12, 1, 8 }, 
    { '~', 0, 0, 9 }, 
    { '!', 13, 1, 10 }, 
    { '=', 14, 1, 11 },
    { '<', 15, 2, 12 }, 
    { '>', 17, 2, 13 }, 
    { '$', 19, 1, 35 } 
};

operand_state operand_chars_1[MAX_OP_STATE_COUNT] = { 
    { '=', 0, 0, 14 }, { '+', 0, 0, 15 },     // +=, ++
    { '=', 0, 0, 16 }, { '-', 0, 0, 17 },     // -=, --
    { '=', 0, 0, 18 },                        // *=
    { '=', 0, 0, 19 },                        // /=
    { '=', 0, 0, 20 },                        // %=
    { '=', 0, 0, 21 },                        // ^=
    { '=', 0, 0, 22 }, { '&', 0, 0, 23 },     // &=, &&
    { '=', 0, 0, 24 }, { '|', 0, 0, 25 },     // |=, ||
    { '=', 0, 0, 26 },                        // #=
    { '=', 0, 0, 27 },                        // !=
    { '=', 0, 0, 28 },                        // ==
    { '=', 0, 0, 29 }, { '<', 0, 1, 30 },     // <=, <<
    { '=', 0, 0, 31 }, { '>', 1, 1, 32 },     // >=, >>
    { '=', 0, 0, 36 } 
};

operand_state operand_chars_2 [ MAX_OP_STATE_COUNT ] = { 
    { '=', 0, 0, 33 }, { '=', 0, 0, 34 } // <<=, >>=
}; 

char delim_chars[MAX_DELIM_COUNT] = {',', '(', ')', '[', ']', '{', '}', ';'};

void lexer::load_source_file(const string& file_name)
{
    FILE* sfile;
    if(!(sfile = fopen(file_name.c_str(), "r")))
    {
        printf("Conuld not open source file for input");
        exit(1);
    }

    char line[MAX_SOURCE_LINE_SIZE + 1];
    while(!feof(sfile))
    {
        line[0] = 0;
        fgets(line, MAX_SOURCE_LINE_SIZE, sfile);
        source_code.push_back(line);
    }

    fclose(sfile);    
}

void lexer::preprocess_source_file()
{
    bool is_in_block_comment = false;
    bool is_in_string = false;

    for(int i = 0; i < source_code.size(); ++i)
    {
        string& line = source_code[i];
        for(int j = 0; j < line.size(); ++j)
        {
            if(line[j] == '"')
            {
                is_in_string = !is_in_string;
            }

            if(line[j] == '/' && line[j + 1] == '/' && !is_in_string && !is_in_block_comment)
            {
                line[j] = '\n';
                line[j + 1] = '\0';
                break;
            }

            if(line[j] == '/' && line[j+1] == '*' && !is_in_string && !is_in_block_comment)
            {
                is_in_block_comment = true;
            }
            if(line[j] == '*' && line[j+1] == '/' && is_in_block_comment)
            {
                line[j] = ' ';
                line[j+1] = ' ';
                is_in_block_comment = false;
            }
            if(is_in_block_comment)
            {
                if(line[j] != '\n')
                {
                    line[j] = ' ';
                }
            }
        }
    }
}

void lexer::reset()
{
    current.line_index = 0;
    current.line = "";
    if(source_code.size() > 0)
    {
        current.line = source_code[current.line_index];
    }

    current.lexeme_start = 0;
    current.lexeme_end = 0;
    current.current_operand = 0;    
}

void lexer::copy_lexer_state(lexer_state& d, lexer_state& s)
{
    d.line_index = s.line_index;
    d.line = s.line;
    d.current_token = s.current_token;
    d.lexeme_start = s.lexeme_start;
    d.lexeme_end = s.lexeme_end;
    d.current_operand = s.current_operand;

    strcpy(d.lexeme, s.lexeme);  
}

int lexer::get_operand_state_index(char c, int cindex, int ssindex, int sscount)
{
    int start_state_index;
    int end_state_index;

    if(cindex == 0)
    {
        start_state_index = 0;
        end_state_index = MAX_OP_STATE_COUNT;
    }
    else
    {
        start_state_index = ssindex;
        end_state_index = ssindex + sscount;
    }

    for(int i = start_state_index; i < end_state_index; ++i)
    {
        char oc;
        switch(cindex)
        {
        case 0:
            oc = operand_chars_0[i].cc;
            break;
        case 1:
            oc = operand_chars_1[i].cc;
            break;
        case 2:
            oc = operand_chars_2[i].cc;
            break;
        }

        if(c == oc)
        {
            return i;
        }
    }

    return -1;
}

bool lexer::is_char_operand_char(char c, int cindex)
{
    for(int i = 0; i < MAX_OP_STATE_COUNT; ++i)
    {
        char oc;
        switch(cindex)
        {
        case 0:
            oc = operand_chars_0[i].cc;
            break;
        case 1:
            oc = operand_chars_1[i].cc;
            break;
        case 2:
            oc = operand_chars_2[i].cc;
            break;
        }

        if(c == oc)
        {
            return true;
        }   
    }

    return false;   
}

operand_state lexer::get_operand_state(int cindex, int sindex)
{
    operand_state os;

    switch(cindex)
    {
    case 0:
        os = operand_chars_0[sindex];
        break;
    case 1:
        os = operand_chars_1[sindex];
        break;
    case 2:
        os = operand_chars_2[sindex];
        break;
    }

    return os;
}

bool lexer::is_char_delim(char c)
{
    for(int i = 0; i < MAX_DELIM_COUNT; ++i)
    {
        if(c == delim_chars[i])
        {
            return true;
        }
    }

    return false;
}

bool lexer::is_char_whitespace(char c)
{
    if(c == ' ' || c == '\t' || c == '\n')
    {
        return true;
    }

    return false;
}

bool lexer::is_char_numeric(char c)
{
    if(c >= '0' && c <= '9')
    {
        return true;
    }

    return false;
}

bool lexer::is_char_identifier(char c)
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

char lexer::get_next_char()
{
    if(current.line.size() == 0)
    {
        return '\0';
    }

    if(current.lexeme_end >= strlen(current.line.c_str()))
    {
        current.line_index++;
        if(current.line_index >= source_code.size())
        {
            return '\0';
        }

        current.lexeme_start = 0;
        current.lexeme_end = 0;
        current.line = source_code[current.line_index];
    }

    return current.line[current.lexeme_end++];
}

char lexer::get_look_ahead_char()
{
    lexer_state prev_state;
    copy_lexer_state(prev_state, current);

    char cc;
    while(true)
    {
        cc = get_next_char();
        if(!is_char_whitespace(cc))
        {
            break;
        }
    }
    copy_lexer_state(current, prev_state);

    return cc;
}

token lexer::get_next_token()
{
    copy_lexer_state(prev, current);

    current.lexeme_start = current.lexeme_end;

    int current_lex_state = LEX_STATE_START;
    
    int current_operand_char_index = 0;
    int current_operand_state_index = 0;
    operand_state current_operand_state;

    bool is_lexeme_done = false;
    bool is_add_current_char = false;
    char current_char;
    int next_lexeme_char_index = 0;

    while(true)
    {
        current_char = get_next_char();
        if(current_char == '\0')
        {
            break;
        }

        is_add_current_char = true;
        switch(current_lex_state)
        {
        case LEX_STATE_UNKNOWN:
            is_lexeme_done = true;
            break;
        case LEX_STATE_START:
            if(is_char_whitespace(current_char))
            {
                ++current.lexeme_start;
                is_add_current_char = false;
            }
            else if(is_char_numeric(current_char))
            {
                current_lex_state = LEX_STATE_INT;
            }
            else if(current_char == '.')
            {
                current_lex_state = LEX_STATE_FLOAT;
            }
            else if(is_char_identifier(current_char))
            {
                current_lex_state = LEX_STATE_IDENT;
            }
            else if(is_char_delim(current_char))
            {
                current_lex_state = LEX_STATE_DELIM;
            }
            else if(is_char_operand_char(current_char, 0))
            {
                current_operand_state_index = get_operand_state_index(current_char, 0, 0, 0);
                if(current_operand_state_index == -1)
                {
                    return TOKEN_TYPE_INVALID;
                }

                current_operand_state = get_operand_state(0, current_operand_state_index);
                current_operand_char_index = 1;
                current.current_operand = current_operand_state.index;

                current_lex_state = LEX_STATE_OP;
            }
            else if(current_char == '"')
            {
                is_add_current_char = false;
                current_lex_state = LEX_STATE_STRING;
            }
            else 
            {
                current_lex_state = LEX_STATE_UNKNOWN;
            }
            break;
        case LEX_STATE_INT:
            if(is_char_numeric(current_char))
            {
                current_lex_state = LEX_STATE_INT;
            }
            else if(current_char == '.')
            {
                current_lex_state = LEX_STATE_FLOAT;
            }
            else if(is_char_whitespace(current_char) || is_char_delim(current_char))
            {
                is_add_current_char = false;
                is_lexeme_done = true;
            }
            else
            {
                current_lex_state = LEX_STATE_UNKNOWN;
            }
            break;
        case LEX_STATE_FLOAT:
            if(is_char_numeric(current_char))
            {
                current_lex_state = LEX_STATE_FLOAT;
            }
            else if(is_char_whitespace(current_char) || is_char_delim(current_char))
            {
                is_lexeme_done = true;
                is_add_current_char = false;
            }
            else
            {
                current_lex_state = LEX_STATE_UNKNOWN;
            }
            break;
        case LEX_STATE_IDENT:
            if(is_char_identifier(current_char))
            {
                current_lex_state = LEX_STATE_IDENT;
            }
            else if(is_char_whitespace(current_char) || is_char_delim(current_char))
            {
                is_add_current_char = false;
                is_lexeme_done = true;
            }
            else
            {
                current_lex_state = LEX_STATE_UNKNOWN;
            }
            break;
        case LEX_STATE_OP:
            if(current_operand_state.sub_state_count == 0)
            {
                is_add_current_char = false;
                is_lexeme_done = true;
                break;
            }
            if(is_char_operand_char(current_char, current_operand_char_index))
            {
                current_operand_state_index = get_operand_state_index(current_char, current_operand_char_index, current_operand_state.sub_state_index, current_operand_state.sub_state_count);
                if(current_operand_state_index == -1)
                {
                    current_lex_state = LEX_STATE_UNKNOWN;
                }
                else
                {
                    current_operand_state = get_operand_state(current_operand_char_index, current_operand_state_index);
                    ++current_operand_char_index;
                    current.current_operand = current_operand_state.index;
                }
            }
            else
            {
                is_add_current_char = false;
                is_lexeme_done = true;
            }
            break;
        case LEX_STATE_DELIM:
            is_add_current_char = false;
            is_lexeme_done = true;
            break;
        case LEX_STATE_STRING:
            if(current_char == '"')
            {
                is_add_current_char = false;
                current_lex_state = LEX_STATE_STRING_CLOSE_QUOTE;
            }
            else if(current_char == '\n')
            {
                is_add_current_char = false;
                current_lex_state = LEX_STATE_UNKNOWN;
            }
            else if(current_char == '\\')
            {
                is_add_current_char = false;
                current_lex_state = LEX_STATE_STRING_ESCAPE;
            }
            break;
        case LEX_STATE_STRING_ESCAPE:
            current_lex_state = LEX_STATE_STRING;
            break;
        case LEX_STATE_STRING_CLOSE_QUOTE:
            is_add_current_char = false;
            is_lexeme_done = true;
            break;
        }//switch(current_lex_state)

        if(is_add_current_char)
        {
            current.lexeme[next_lexeme_char_index] = current_char;
            ++next_lexeme_char_index;
        }

        if(is_lexeme_done)
        {
            break;
        }
    }

    current.lexeme[next_lexeme_char_index] = '\0';
    --current.lexeme_end;

    token tt;
    switch(current_lex_state)
    {
    case LEX_STATE_UNKNOWN:
        tt = TOKEN_TYPE_INVALID;
        break;
    case LEX_STATE_INT:
        tt = TOKEN_TYPE_INT;
        break;
    case LEX_STATE_FLOAT:
        tt = TOKEN_TYPE_FLOAT;
        break;
    case LEX_STATE_IDENT:
        tt = TOKEN_TYPE_IDENT;
        if(strcmp(current.lexeme, "var") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_VAR;
        }
        if(strcmp(current.lexeme, "true") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_TRUE;
        }
        if(strcmp(current.lexeme, "false") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_FALSE;
        }
        if(strcmp(current.lexeme, "if") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_IF;
        }
        if(strcmp(current.lexeme, "else") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_ELSE;
        }
        if(strcmp(current.lexeme, "break") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_BREAK;
        }
        if(strcmp(current.lexeme, "continue") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_CONTINUE;
        }
        if(strcmp(current.lexeme, "for") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_FOR;
        }
        if(strcmp(current.lexeme, "while") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_WHILE;
        }
        if(strcmp(current.lexeme, "function") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_FUNC;
        }
        if(strcmp(current.lexeme, "return") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_RETURN;
        }
        if(strcmp(current.lexeme, "host") == 0)
        {
            tt = TOKEN_TYPE_RSRVD_HOST;
        }
        break;
    case LEX_STATE_DELIM:
        switch(current.lexeme[0])
        {
        case ',':
            tt = TOKEN_TYPE_DELIM_COMMA;
            break;
        case '(':
            tt = TOKEN_TYPE_DELIM_OPEN_PAREN;
            break;
        case ')':
            tt = TOKEN_TYPE_DELIM_CLOSE_PAREN;
            break;
        case '[':
            tt = TOKEN_TYPE_DELIM_OPEN_BRACE;
            break;
        case ']':
            tt = TOKEN_TYPE_DELIM_CLOSE_BRACE;
            break;
        case '{':
            tt = TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE;
            break;
        case '}':
            tt = TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE;
            break;
        case ';':
            tt = TOKEN_TYPE_DELIM_SEMICOLON;
            break;
        }//current.lexeme[0]
        break;
    case LEX_STATE_OP:
        tt = TOKEN_TYPE_OP;
        break;
    case LEX_STATE_STRING_CLOSE_QUOTE:
        tt = TOKEN_TYPE_STRING;
        break;
    default:
        tt = TOKEN_TYPE_END_OF_STREAM;
    }//switch(current_lex_state)

    current.current_token = tt;
    return tt;      
}

}//namespace xcomplier
}//namespace xscript
