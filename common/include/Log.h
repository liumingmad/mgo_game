#ifndef LOG_H
#define LOG_H

#include <iostream>

class Log
{
private:
    Log();
    ~Log();
public:
    static void info(std::string message);
    static void error(std::string message);
};

#endif // LOG_H