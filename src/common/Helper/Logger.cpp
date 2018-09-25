
#include <iostream>
#include "Logger.h"

std::string Logger::consoleTranslation[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
std::function<void(Logger::LogLevel, std::string)> Logger::_callback = Logger::defaultCallback;
Logger::LogLevel Logger::_minLogLevel = LogLevel::DEBUG;


void Logger::defaultCallback(LogLevel level, std::string value)
{
    std::stringstream str;
    str << "Level: " << consoleTranslation[level] << " Message: " << value << std::endl;
    std::cout << str.str() << std::flush;
}
