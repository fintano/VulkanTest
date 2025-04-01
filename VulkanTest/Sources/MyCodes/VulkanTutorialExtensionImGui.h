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

	// TextureType 열거형 정의
	enum class TextureType {
		Albedo = 0,
		Normal = 1,
		Metallic = 2,
		Roughness = 3,
		AO = 4
	};

	// 재질 텍스처 정보 구조체
	struct MaterialTextureInfo {
		std::string path;          // 텍스처 파일 경로
		std::string displayName;   // UI에 표시할 이름
		TextureType type;          // 텍스처 타입
		int viewerIndex;           // TextureViewer에서의 인덱스
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

		/** related to Model Tabl*/
		void SetModelLoadCallback(ModelLoadCallback callback) { m_modelLoadCallback = callback; }
		void SetModelRemoveCallback(ModelLoadCallback callback) { m_modelRemoveCallback = callback; }
		void SetModelTransformChangeCallback(ModelTransformChangeCallback callback) { m_modelTransformChangeCallback = callback; }
		ModelTransformChangeCallback getModelTransformChangeCallback() { return m_modelTransformChangeCallback; }
		void SetModelLoadResult(bool success, const std::string& modelName = "");
		void InitializeTransformEditor(int modelIndex);
		void SetModelTransform(int modelIndex, const ImGui::ModelTransform& transform);
		void SetModelTransform(int modelIndex, const glm::mat4& mat);

		/** related to Material Tab*/
		void SetExtension(VulkanTutorialExtension* extension) { m_extension = extension; }
		using CreateMaterialCallback = std::function<void(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&)>;
		void SetCreateMaterialCallback(CreateMaterialCallback callback) { m_createMaterialCallback = callback; }
		using MaterialModelChangeCallback = std::function<void(int)>;
		void SetMaterialModelChangeCallback(MaterialModelChangeCallback callback) { m_materialModelChangeCallback = callback; }

	private:
		// 탭 컨텐츠 렌더링 함수 (구현은 cpp 파일에서)
		void RenderModelTab();
		void RenderMaterialTab();
		void DisplayTextureLoadUI(const char* labelName, std::string& texturePath, std::function<void(const std::string&)> onTextureSelected);
		std::string GetTextureTypeName(TextureType type);
		void OpenTextureFileBrowser(TextureType type);
		void HandleTextureFileBrowser();
		void LoadCustomTexture(const std::string& path, TextureType type);

		// 상태 정보
		LeftPanelState m_state;
		ModelLoadCallback m_modelLoadCallback;
		ModelLoadCallback m_modelRemoveCallback;
		ModelTransformChangeCallback m_modelTransformChangeCallback;

		std::vector<std::string> m_loadedModels;
		std::unordered_map<int, TransformEditor> m_modelTransformEditors;
		int m_selectedModelIndex = -1;
		const char* m_fileDialogKey = "ChooseGLTFModelDlgKey";
		TextureType m_currentSelectingTextureType = TextureType::Albedo;

		// Material 탭 멤버 변수들
		VulkanTutorialExtension* m_extension = nullptr;
		std::string m_fileDialogKeyMaterial = "MaterialTextureDialog";

		std::string m_albedoPath = "";
		std::string m_normalPath = "";
		std::string m_metallicPath = "";
		std::string m_roughnessPath = "";
		std::string m_aoPath = "";
		std::string m_materialName = "viewer";

		// 재질 생성 콜백
		CreateMaterialCallback m_createMaterialCallback = nullptr;
		MaterialModelChangeCallback m_materialModelChangeCallback = nullptr;
		// 모델 선택 관련 변수
		int m_useBoxModel = 1; // 0 = Box, 1 = Sphere

		using TextureSelectedCallback = std::function<void(const std::string&)>;
		TextureSelectedCallback m_currentSelectingTextureCallback = nullptr;
		std::string m_lastBrowsedDirectory = "";
		std::string m_currentSelectingTexturePath = "";
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
	};
}