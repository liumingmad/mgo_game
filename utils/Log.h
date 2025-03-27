#ifndef LOG_H
#define LOG_H

#include <iostream>

class Log
{
private:
    Log();
    ~Log();
public:
    static void info(const char* message);
    static void error(const char* message);
};

#endif // LOG_H