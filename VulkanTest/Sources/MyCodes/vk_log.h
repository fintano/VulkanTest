#pragma once
#include <iostream>
#include <sstream>
#include <string>

enum LogLevel { Log, Display, Warning, Error };
extern LogLevel currentLevel;

// �Լ� ����
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

    // Visual Studio Output â�� ���
    OutputToDebugConsole(fullMessage);

    // DISPLAY ���� �̻��� �ֿܼ��� ���
    if (level >= Display) {
        // ���� ����
        if (level == Warning) {
            SetConsoleColor(6); // ����� (Windows.h ��� ����)
        }
        else if (level == Error) {
            SetConsoleColor(4); // ������ (Windows.h ��� ����)
        }
        std::cout << fullMessage << std::endl;
        // ���� �������
        SetConsoleColor(7); // �⺻ ����
    }
}

#define LOG(level, fmt, ...) \
    if (level >= currentLevel) { \
        LogMessage(level, fmt, ##__VA_ARGS__); \
    }