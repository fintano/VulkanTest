#include "vk_log.h"
#include <Windows.h>

LogLevel currentLevel = Log;

void SetConsoleColor(unsigned short color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void OutputToDebugConsole(const std::string& message) {
    OutputDebugStringA((message + "\n").c_str());
}