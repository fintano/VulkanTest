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

// 왼쪽 패널 상태 구조체
struct LeftPanelState {
    enum Tab {
        ModelTab = 0,
        MaterialTab = 1
    };

    Tab selectedTab = ModelTab;
    Tab prevSelectedTab = ModelTab;

    // 스크롤 상태
    float modelScroll = 0.0f;
    float materialScroll = 0.0f;

    bool modelLoaded = false;
    std::string lastLoadedModelPath;
    bool requestLoadModel = false;
};

// 왼쪽 패널 UI 관리자
class LeftPanelUI {
public:
    LeftPanelUI();
    ~LeftPanelUI();

    // 패널 렌더링 함수
    void Render(int x, int y, int width, int height, bool enabled = true);

    // 상태 접근
    LeftPanelState& GetState() { return m_state; }

    using ModelLoadCallback = std::function<void(const std::string&)>;
    void SetModelLoadCallback(ModelLoadCallback callback) { m_modelLoadCallback = callback; }
    void SetModelLoadResult(bool success, const std::string& modelName = "");

private:
    // 탭 컨텐츠 렌더링 함수 (구현은 cpp 파일에서)
    void RenderModelTab();
    void RenderMaterialTab();

    // 상태 정보
    LeftPanelState m_state;
    ModelLoadCallback m_modelLoadCallback;

    std::vector<std::string> m_loadedModels;
    int m_selectedModelIndex = -1;
};

// 오른쪽 패널 UI 관리자
class RightPanelUI {
public:
    RightPanelUI();
    ~RightPanelUI();

    // 패널 렌더링 함수
    void Render(int x, int y, int width, int height, bool enabled = true);

    // VulkanTutorialExtension 참조 설정
    void SetVulkanExtension(VulkanTutorialExtension* extension) { m_extension = extension; }

private:
    // 컨텐츠 렌더링 함수
    void RenderContent();

    // Vulkan Extension 참조
    VulkanTutorialExtension* m_extension = nullptr;

    // 창 관련 상태
    bool m_open = true;
};