#include "Logger.h"

#include <sstream>
#include <iomanip>

Logger::Logger() {
    logFile.open("logs.txt", std::ios_base::out);
}

Logger::~Logger() {
    logFile.close();
}

void Logger::log(u16 address) {
    logFile << u16ToHex(address) << ": ";
}

void Logger::log(const std::string& opCode, const std::string& args, const std::string& info) {
    logFile << opCode;

    if (args.length() > 0)
        logFile << " " << args;

    if (info.length() > 0)
        logFile << " ; " << info;

    logFile << std::endl;
}

void Logger::log(const std::string& message) {
    logFile << message;
}

std::string Logger::u8ToHex(u8 value) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << std::hex << (u16)value;
    return stream.str();
}

std::string Logger::u16ToHex(u16 value) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(4) << std::hex << value;
    return stream.str();
}

std::string Logger::s8ToString(s8 value) {
    return std::to_string(value);
}

std::string Logger::u8ToString(u8 value) {
    return std::to_string(value);
}