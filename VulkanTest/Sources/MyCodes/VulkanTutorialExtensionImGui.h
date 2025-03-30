#pragma once

#include <string>
#include <vector>
#include <functional>

class VulkanTutorialExtension;

namespace ImGui
{
	bool CanUseDirectionalLight();
	void PrintDebugString();

	extern std::string stringToDebug;
}

// ���� �г� ���� ����ü
struct LeftPanelState {
    enum Tab {
        ModelTab = 0,
        MaterialTab = 1
    };

    Tab selectedTab = ModelTab;
    Tab prevSelectedTab = ModelTab;

    // ��ũ�� ����
    float modelScroll = 0.0f;
    float materialScroll = 0.0f;

    bool modelLoaded = false;
    std::string lastLoadedModelPath;
    bool requestLoadModel = false;
};

// ���� �г� UI ������
class LeftPanelUI {
public:
    LeftPanelUI();
    ~LeftPanelUI();

    // �г� ������ �Լ�
    void Render(int x, int y, int width, int height, bool enabled = true);

    // ���� ����
    LeftPanelState& GetState() { return m_state; }

    using ModelLoadCallback = std::function<void(const std::string&)>;
    void SetModelLoadCallback(ModelLoadCallback callback) { m_modelLoadCallback = callback; }
    void SetModelLoadResult(bool success, const std::string& modelName = "");

private:
    // �� ������ ������ �Լ� (������ cpp ���Ͽ���)
    void RenderModelTab();
    void RenderMaterialTab();

    // ���� ����
    LeftPanelState m_state;
    ModelLoadCallback m_modelLoadCallback;

    std::vector<std::string> m_loadedModels;
    int m_selectedModelIndex = -1;
};

// ������ �г� UI ������
class RightPanelUI {
public:
    RightPanelUI();
    ~RightPanelUI();

    // �г� ������ �Լ�
    void Render(int x, int y, int width, int height, bool enabled = true);

    // VulkanTutorialExtension ���� ����
    void SetVulkanExtension(VulkanTutorialExtension* extension) { m_extension = extension; }

private:
    // ������ ������ �Լ�
    void RenderContent();

    // Vulkan Extension ����
    VulkanTutorialExtension* m_extension = nullptr;

    // â ���� ����
    bool m_open = true;
};