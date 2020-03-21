#pragma once

#include "Types.h"
#include <fstream>
#include <sstream>

class Logger {
public:
    Logger();
    virtual ~Logger();
    void log(u16 address);
    void log(const std::string& opCode, const std::string& args, const std::string& info);
    void log(const std::string& message);

    std::string u8ToHex(u8 value);
    std::string u16ToHex(u16 value);
    std::string u8ToString(u8 value);
    std::string s8ToString(s8 value);

private:
    std::ofstream logFile;
	std::stringstream stream;
};
