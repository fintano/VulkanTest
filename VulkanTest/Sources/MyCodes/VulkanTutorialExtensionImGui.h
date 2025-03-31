#pragma once

#include "vk_types.h"

#include <string>
#include <vector>
#include <functional>

class VulkanTutorialExtension;
class TextureViewer;

namespace ImGui
{
	class LeftPanelUI;
	class RightPanelUI;

	typedef int ImGuiTreeNodeFlags;

	bool CanUseDirectionalLight();
	void PrintDebugString();

	extern std::string stringToDebug;

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

	struct ModelTransform {

		ModelTransform();
		ModelTransform(glm::mat4 mat);

		float position[3] = { 0.0f, 0.0f, 0.0f };
		float rotation[3] = { 0.0f, 0.0f, 0.0f };
		float scale[3] = { 1.0f, 1.0f, 1.0f };
	public:
		glm::mat4 matrix() const;
	};

	// Transform 편집 클래스
	class TransformEditor {
	public:

		TransformEditor() = default;
		~TransformEditor() = default;

		void SetParent(const std::shared_ptr<LeftPanelUI>& parent) {
			m_parent = parent;
		}

		// Transform 값 설정
		void SetTransform(const ModelTransform& transform) {
			m_transform = transform;
		}

		// 현재 Transform 값 가져오기
		const ModelTransform& GetTransform() const {
			return m_transform;
		}

		bool Render(int selectedIndex, const char* label = "Transform");

		void Reset();

	private:
		std::weak_ptr<LeftPanelUI> m_parent;
		ModelTransform m_transform;
	};

	// 왼쪽 패널 UI 관리자
	class LeftPanelUI : public std::enable_shared_from_this<LeftPanelUI> {
	public:
		LeftPanelUI();
		~LeftPanelUI();

		// 패널 렌더링 함수
		void Render(int x, int y, int width, int height, bool enabled = true);

		bool RemoveModel(int index);

		// 상태 접근
		LeftPanelState& GetState() { return m_state; }

		using ModelLoadCallback = std::function<void(const std::string&)>;
		using ModelTransformChangeCallback = std::function<void(int modelIndex, const ImGui::ModelTransform& transform)>;

		void SetModelLoadCallback(ModelLoadCallback callback) { m_modelLoadCallback = callback; }
		void SetModelRemoveCallback(ModelLoadCallback callback) { m_modelRemoveCallback = callback; }
		void SetModelTransformChangeCallback(ModelTransformChangeCallback callback) { m_modelTransformChangeCallback = callback; }
		ModelTransformChangeCallback getModelTransformChangeCallback() { return m_modelTransformChangeCallback; }
		void SetModelLoadResult(bool success, const std::string& modelName = "");
		void InitializeTransformEditor(int modelIndex);
		void SetModelTransform(int modelIndex, const ImGui::ModelTransform& transform);
		void SetModelTransform(int modelIndex, const glm::mat4& mat);

	private:
		// 탭 컨텐츠 렌더링 함수 (구현은 cpp 파일에서)
		void RenderModelTab();
		void RenderMaterialTab();


		// 상태 정보
		LeftPanelState m_state;
		ModelLoadCallback m_modelLoadCallback;
		ModelLoadCallback m_modelRemoveCallback;
		ModelTransformChangeCallback m_modelTransformChangeCallback;

		std::vector<std::string> m_loadedModels;
		std::unordered_map<int, TransformEditor> m_modelTransformEditors;
		int m_selectedModelIndex = -1;
		const char* m_fileDialogKey = "ChooseGLTFModelDlgKey";
	};

	// 오른쪽 패널 UI 관리자
	class RightPanelUI : public std::enable_shared_from_this<RightPanelUI> {
	public:
		RightPanelUI(VulkanTutorialExtension* extension);
		~RightPanelUI();

		// 패널 렌더링 함수
		void Render(int x, int y, int width, int height, bool enabled = true);

		// VulkanTutorialExtension 참조 설정
		void SetTextureViewer(const std::shared_ptr<TextureViewer>& textureViewer) { m_textureViewer = textureViewer; }

	private:
		// 컨텐츠 렌더링 함수
		void RenderContent();

		// 텍스처 뷰어 섹션 렌더링을 위한 새 메서드
		void RenderTextureViewer();

		// Vulkan Extension 참조
		VulkanTutorialExtension* m_extension = nullptr;
		std::shared_ptr<TextureViewer> m_textureViewer;

		// 창 관련 상태
		bool m_open = true;

		int m_selectedTextureIndex = -1;
		bool m_showTextureViewer = true;
	};
}