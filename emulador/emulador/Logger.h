#pragma once

#include "Types.h"
#include <fstream>
#include <sstream>

class Logger {
public:
    static Logger* instance;

    Logger();
    virtual ~Logger();
    void log(u16 address);
    void log(const std::string& args, const std::string& info);
    void log(const std::string& message);

    static std::string u8ToHex(u8 value);
    static std::string u16ToHex(u16 value);
    static std::string u8ToString(u8 value);
    static std::string s8ToString(s8 value);

private:
    std::ofstream logFile;
	std::stringstream stream;
};
