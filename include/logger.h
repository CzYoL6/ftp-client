#pragma once

#include <iostream>

#define NONE                 "\e[0m"
#define BLACK                "\e[0;30m"
#define L_BLACK              "\e[1;30m"
#define RED                  "\e[0;31m"
#define L_RED                "\e[1;31m"
#define GREEN                "\e[0;32m"
#define L_GREEN              "\e[1;32m"
#define BROWN                "\e[0;33m"
#define YELLOW               "\e[1;33m"
#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
#define PURPLE               "\e[0;35m"
#define L_PURPLE             "\e[1;35m"
#define CYAN                 "\e[0;36m"
#define L_CYAN               "\e[1;36m"
#define GRAY                 "\e[0;37m"
#define WHITE                "\e[1;37m"
 
#define BOLD                 "\e[1m"
#define UNDERLINE            "\e[4m"
#define BLINK                "\e[5m"
#define REVERSE              "\e[7m"
#define HIDE                 "\e[8m"
#define CLEAR                "\e[2J"
#define CLRLINE              "\r\e[K"




// pure console log:
// #define PRINT(color, tag, format, arg...) do{printf(color " " tag NONE " " format, ## arg);}while(0)
// #define LOGERR(msg, arg...) PRINT( RED, "[ ERROR ]",  "(%s:%d)   " msg " \n" , __FILE__, __LINE__, ## arg)
// #define LOGMSG(msg, arg...) PRINT( GREEN, "[MESSAGE]", "(%s:%d)   " msg "\n"  , __FILE__, __LINE__, ## arg)

//console + gui

class ILogger{
public:
    virtual void AddLog(const char* fmt, ...) = 0;
};

extern ILogger* p_logger;
#define PRINTGUI(format, arg...) do{ if(p_logger) p_logger->AddLog(format, ## arg); }while(0)
#define PRINT(color, tag, format, arg...) do{printf(color " " tag NONE " " format, ## arg);}while(0)
#define LOGERR(msg, arg...) do{PRINTGUI(msg, ## arg) ; \
                                        PRINT( RED, "[ ERROR ]",  "(%s:%d)   " msg " \n" , __FILE__, __LINE__, ## arg);}while(0)
#define LOGMSG(msg, arg...) do{PRINTGUI(msg, ## arg) ; \
                                        PRINT( GREEN, "[MESSAGE]", "(%s:%d)   " msg "\n"  , __FILE__, __LINE__, ## arg);}while(0)

