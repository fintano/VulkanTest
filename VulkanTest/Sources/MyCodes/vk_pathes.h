#pragma once
#include <Windows.h>

#include <string>
#include <filesystem>
#include <iostream>

namespace Utils {
    static const std::string ProjectName = "VulkanTest";

    std::filesystem::path GetProjectRoot();
}