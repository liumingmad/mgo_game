#include "Log.h"

void Log::info(const char *message)
{
    std::cout << "Info: " << message << std::endl;
}

void Log::error(const char *message)
{
    std::cout << "Error: " << message << std::endl;
}