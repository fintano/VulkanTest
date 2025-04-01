#pragma once
#include <iostream>
#include <sstream>
#include <string>

enum LogLevel { Log, Display, Warning, Error };
extern LogLevel currentLevel;

// 함수 선언
void SetConsoleColor(unsigned short color);
void OutputToDebugConsole(const std::string& message);

template<typename... Args>
std::string FormatString(const char* fmt, Args&&... args) {
    std::string format(fmt);
    size_t pos = 0;
    ([&] {
        pos = format.find("{}", pos);
        if (pos != std::string::npos) {
            std::stringstream ss;
            ss << args;
            format.replace(pos, 2, ss.str());
        }
        }(), ...);
    return format;
}

template<typename... Args>
void LogMessage(LogLevel level, const char* fmt, Args&&... args) {
    static const char* levelNames[] = { "LOG", "DISPLAY", "WARNING", "ERROR" };
    std::string message = FormatString(fmt, std::forward<Args>(args)...);
    std::string fullMessage = "[" + std::string(levelNames[level]) + "] " + message;

    // Visual Studio Output 창에 출력
    OutputToDebugConsole(fullMessage);

    // DISPLAY 레벨 이상은 콘솔에도 출력
    if (level >= Display) {
        // 색상 설정
        if (level == Warning) {
            SetConsoleColor(6); // 노란색 (Windows.h 상수 없이)
        }
        else if (level == Error) {
            SetConsoleColor(4); // 빨간색 (Windows.h 상수 없이)
        }
        std::cout << fullMessage << std::endl;
        // 색상 원래대로
        SetConsoleColor(7); // 기본 색상
    }
}

#define LOG(level, fmt, ...) \
    if (level >= currentLevel) { \
        LogMessage(level, fmt, ##__VA_ARGS__); \
    }