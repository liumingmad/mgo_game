#include "Log.h"

void Log::info(std::string message)
{
    std::cout << "Info: " << message << std::endl;
}

void Log::error(std::string message)
{
    std::cout << "Error: " << message << std::endl;
}