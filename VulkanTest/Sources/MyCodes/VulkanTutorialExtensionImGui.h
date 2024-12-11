#pragma once

#include <string>

class VulkanTutorialExtension;

namespace ImGui
{
	void ShowVulkanWindow(bool* p_open = nullptr);
	bool CanUseDirectionalLight();
	void PrintDebugString();

	extern std::string stringToDebug;
}