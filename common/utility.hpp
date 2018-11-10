#ifndef     __XSCRIPT_UTILITY_HPP__
#define     __XSCRIPT_UTILITY_HPP__

#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

inline unsigned long get_tick_count()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

inline char* string_to_upper(char* str)
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

inline int kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
       ungetc(ch, stdin);
       return 1;
    }
    
    return 0;
}

#endif      //__XSCRIPT_UTILITY_HPP__
