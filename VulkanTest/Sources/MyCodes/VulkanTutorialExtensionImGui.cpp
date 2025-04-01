#include <algorithm>
#include "VulkanTutorialExtensionImGui.h"
#include "VulkanTutorialExtension.h"
#include "UniformBufferTypes.h"
#include "TextureViewer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuiFileDialog.h"
#include "MaterialTester.h"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

void VulkanTutorialExtension::initImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
}

void VulkanTutorialExtension::createImGui()
{
	if (!findQueueFamilies(physicalDevice).graphicsFamily.has_value())
	{
		throw std::runtime_error("failed to find graphics queue familites!");
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = physicalDevice;
	init_info.Device = getDevice();
	init_info.QueueFamily = *(findQueueFamilies(physicalDevice).graphicsFamily);
	init_info.Queue = graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = imGuiDescriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = swapChainImages.size();
	init_info.ImageCount = swapChainImages.size();
	init_info.CheckVkResultFn = check_vk_result;
	init_info.RenderPass = imGuiRenderPass;
	init_info.Subpass = 0;
	ImGui_ImplVulkan_Init(&init_info);
}

void VulkanTutorialExtension::createImGuiDescriptorPool()
{
	// ImGui DescriptorPool�� �ѹ��� �����.
	if (imGuiDescriptorPool)
	{
		return;
	}

	static const VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(sizeof(poolSizes) / sizeof(VkDescriptorPoolSize));
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size()) * 10;
	// ImGui ���ο��� Pool���� �Ҵ�� DescriptorSet ������ �޸� �����Ѵ�. �׷��� �� �÷��װ� �ʿ��ϴ�. 
	// ���������� Pool ������ ���� ���ش�.
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(getDevice(), &poolInfo, nullptr, &imGuiDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanTutorialExtension::drawImGui(uint32_t imageIndex)
{
	// â ũ�� ���
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float windowWidth = viewport->Size.x;
	const float windowHeight = viewport->Size.y;
	const float PanelWidth = windowWidth * 0.2f;
	const float rightPanelX = windowWidth - PanelWidth;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();

	static bool leftPanelInitialized = false;

	if (!leftPanelInitialized) {
		leftPanel->SetModelLoadCallback([this](const std::string& modelPath) {
			loadGltfModel(modelPath);
			});
		leftPanel->SetModelRemoveCallback([this](const std::string& modelPath) {
			removeGltfModelDeferred(modelPath);
			});
		leftPanel->SetModelTransformChangeCallback([this](int modelIndex, const ImGui::ModelTransform& transform) {
			onChangedGltfModelTransform(modelIndex, transform);
			});
		leftPanel->SetCreateMaterialCallback([this](const std::string& name, const std::string& albedo, const std::string& normal, const std::string& metallic, const std::string& roughness, const std::string& ao) {
			materialTester->createMaterial(this, name, albedo.c_str(), normal.c_str(), metallic.c_str(), roughness.c_str(), ao.c_str());
			});
		leftPanel->SetMaterialModelChangeCallback([this](int model) {
			materialTester->selectModel(static_cast<MaterialTester::Model>(model));
			});
		leftPanelInitialized = true;
	}
	leftPanel->Render(0, 0, PanelWidth, windowHeight);

	static bool rightPanelinitialized = false;

	rightPanel->SetTextureViewer(textureViewer);

	// ������ �г� ������
	rightPanel->Render(rightPanelX, 0, PanelWidth, windowHeight);

	ImGui::Render();


	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkResult err = vkBeginCommandBuffer(imGuiCommandBuffers[imageIndex], &info);
	check_vk_result(err);

	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = imGuiRenderPass;
		info.framebuffer = imGuiFrameBuffers[imageIndex];
		info.renderArea.extent = swapChainExtent;

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 1.f } };

		info.clearValueCount = static_cast<uint32_t>(clearValues.size());
		info.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(imGuiCommandBuffers[imageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imGuiCommandBuffers[imageIndex]);

	// Submit command buffer
	vkCmdEndRenderPass(imGuiCommandBuffers[imageIndex]);
	err = vkEndCommandBuffer(imGuiCommandBuffers[imageIndex]);
	check_vk_result(err);

	addCommandBuffer(imGuiCommandBuffers[imageIndex]);
}

void VulkanTutorialExtension::createImGuiCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(getDevice(), &commandPoolCreateInfo, nullptr, &imGuiCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("Could not create graphics command pool");
	}
}

void VulkanTutorialExtension::createImGuiCommandBuffers()
{
	imGuiCommandBuffers.resize(swapChainImages.size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = imGuiCommandPool;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)imGuiCommandBuffers.size();
	if (vkAllocateCommandBuffers(getDevice(), &commandBufferAllocateInfo, imGuiCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers.");
	}
}

void VulkanTutorialExtension::createImGuiFrameBuffers()
{
	imGuiFrameBuffers.resize(swapChainImageViews.size());

	VkImageView attachment[1];
	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = imGuiRenderPass;
	info.attachmentCount = 1;
	info.pAttachments = attachment;
	info.width = swapChainExtent.width;
	info.height = swapChainExtent.height;
	info.layers = 1;

	for (uint32_t i = 0; i < swapChainImageViews.size(); i++)
	{
		attachment[0] = swapChainImageViews[i];
		VkResult err = vkCreateFramebuffer(getDevice(), &info, nullptr, &imGuiFrameBuffers[i]);
		check_vk_result(err);
	}
}

void VulkanTutorialExtension::createImGuiRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// �̷��� �Ϸ��� resolve attachment�� finalLayout�� present�� �ƴ� ������ �����ؾߵ��� ������?
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &colorAttachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;
	if (vkCreateRenderPass(getDevice(), &info, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Could not create Dear ImGui's render pass");
	}
}

void VulkanTutorialExtension::cleanUpImGuiSwapchain()
{
	// Resources to destroy on swapchain recreation
	for (auto framebuffer : imGuiFrameBuffers) {
		vkDestroyFramebuffer(getDevice(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(getDevice(), imGuiRenderPass, nullptr);
}

void VulkanTutorialExtension::cleanUpImGui()
{
	// Resources to destroy when the program ends
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(getDevice(), imGuiDescriptorPool, nullptr);
	vkFreeCommandBuffers(getDevice(), imGuiCommandPool, static_cast<uint32_t>(imGuiCommandBuffers.size()), imGuiCommandBuffers.data());
	vkDestroyCommandPool(getDevice(), imGuiCommandPool, nullptr);
}

namespace ImGui
{
	std::string getFileName(const std::string& path)
	{
		// ���ϸ� ����
		std::string fileName = path;
		size_t lastSlash = fileName.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			fileName = fileName.substr(lastSlash + 1);
		}
		return fileName;
	}

	std::string ImGui::stringToDebug;

	void ImGui::PrintDebugString()
	{
		if (!stringToDebug.empty())
		{
			ImGui::Text("%s", stringToDebug.c_str());
			ImGui::Spacing();
		}
	}

	LeftPanelUI::LeftPanelUI() {
		// �ʱ�ȭ �ڵ�
	}

	LeftPanelUI::~LeftPanelUI() = default;

	void LeftPanelUI::Render(int x, int y, int width, int height, bool enabled) {
		// ������ ����
		ImGui::SetNextWindowPos(ImVec2(float(x), float(y)));
		ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse;

		if (!enabled) {
			window_flags |= ImGuiWindowFlags_NoInputs;
		}

		// �г� ����
		ImGui::Begin("LeftPanel", nullptr, window_flags);

		// �� �� ����
		if (ImGui::BeginTabBar("LeftPanelTabs")) {
			// �� ��
			if (ImGui::BeginTabItem("Model")) {
				m_state.prevSelectedTab = m_state.selectedTab;
				m_state.selectedTab = LeftPanelState::ModelTab;

				RenderModelTab();

				ImGui::EndTabItem();
			}

			// ���� ��
			if (ImGui::BeginTabItem("Material")) {
				m_state.prevSelectedTab = m_state.selectedTab;
				m_state.selectedTab = LeftPanelState::MaterialTab;

				RenderMaterialTab();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	bool LeftPanelUI::RemoveModel(int index) {
		if (index < 0 || index >= m_loadedModels.size()) {
			return false;
		}

		m_modelRemoveCallback(m_loadedModels[index]);

		// �� ��� ��Ͽ��� ����
		m_loadedModels.erase(m_loadedModels.begin() + index);

		// Transform ������ ����
		m_modelTransformEditors.erase(index);

		// ���õ� �ε��� ����
		if (m_selectedModelIndex == index) {
			// ���ŵ� �׸��� ���õǾ��� ���̸� ���� �ʱ�ȭ
			m_selectedModelIndex = (m_loadedModels.empty()) ? -1 : 0;
		}
		else if (m_selectedModelIndex > index) {
			// ���ŵ� �׸񺸴� �ڿ� �ִ� ���� �ε��� ����
			m_selectedModelIndex--;
		}

		return true;
	}

	void LeftPanelUI::RenderModelTab() {
		// ��� ������ �Լ� - ���Ʒ� ���м� �߰�
		auto renderHeaderWithLines = [](const char* label) {
			// ��� ���м� �߰�
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// ��� �ؽ�Ʈ ���� ����
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.95f, 1.0f, 1.0f)); // �� ���� ��� �迭

			// ��� ������
			bool isOpen = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);

			// �ؽ�Ʈ ���� ����
			ImGui::PopStyleColor();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			return isOpen;
			};

		// ��ü ����� ���� �⺻ ��Ÿ�� ���� (�� £�� ������ �Ķ��� �迭)
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.35f, 0.4f, 0.8f));         // �ణ �� ���� û�ϻ�
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.45f, 0.55f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.35f, 0.5f, 0.65f, 0.8f));

		if (renderHeaderWithLines("Load Model")) {
			ImGui::Text("Load a 3D model in GLTF format");

			if (ImGui::Button("Browse GLTF Model...", ImVec2(-1, 0))) {
				// �ùٸ� FileDialogConfig ���
				IGFD::FileDialogConfig config;
				config.path = ".";
				config.flags = ImGuiFileDialogFlags_Modal;

				ImGuiFileDialog::Instance()->OpenDialog(
					m_fileDialogKey,
					"Choose GLTF Model",
					"gltf;glb{.gltf,.glb}",
					config);
			}

			// ���⼭ ���ϴ� ũ��� ǥ��
			ImVec2 maxSize = ImGui::GetMainViewport()->Size;
			maxSize.x *= 0.4f;
			maxSize.y *= 0.4f;
			ImVec2 minSize = maxSize;

			// ���� ���̾�α� ǥ�� �� ��� ó��
			if (ImGuiFileDialog::Instance()->Display(m_fileDialogKey, ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

					if (m_modelLoadCallback) {
						m_modelLoadCallback(filePathName);
					}
				}
				ImGuiFileDialog::Instance()->Close();
			}
		}

		if (renderHeaderWithLines("Models")) {
			if (m_loadedModels.empty()) {
				ImGui::Text("No models loaded yet.");
				// �⺻ ��Ÿ�� ����
				ImGui::PopStyleColor(3);
			}
			else {
				// �⺻ ��Ÿ�� ����
				ImGui::PopStyleColor(3);

				ImGui::BeginChild("ModelsList", ImVec2(0, 200), true);

				// ������ �� �ε��� ���� ����
				int modelToRemove = -1;

				for (int i = 0; i < m_loadedModels.size(); i++) {
					// ���ϸ� ����
					std::string fileName = getFileName(m_loadedModels[i]);

					ImGui::PushID(i);

					// �׸� ���̿� �ʺ� ���
					float lineHeight = ImGui::GetTextLineHeightWithSpacing();
					float itemWidth = ImGui::GetContentRegionAvail().x - 30;

					// ���� ������ ����
					bool isSelected = (m_selectedModelIndex == i);
					if (ImGui::Selectable(fileName.c_str(), &isSelected, 0, ImVec2(itemWidth, lineHeight))) {
						m_selectedModelIndex = i;
					}

					// ���� ��ư
					ImGui::SameLine(itemWidth + 20);
					if (ImGui::Button("X", ImVec2(20, lineHeight))) {
						modelToRemove = i;
					}

					ImGui::PopID();
				}

				// ���� ���� ���� ó��
				if (modelToRemove >= 0) {
					RemoveModel(modelToRemove);
				}

				ImGui::EndChild();
			}
		}

		if (m_selectedModelIndex >= 0) {
			TransformEditor& transformEditor = m_modelTransformEditors[m_selectedModelIndex];
			transformEditor.Render(m_selectedModelIndex, m_loadedModels[m_selectedModelIndex].c_str());
		}
	}

	void LeftPanelUI::SetModelLoadResult(bool success, const std::string& modelPath) {
		if (success) {
			m_state.modelLoaded = true;
			m_state.lastLoadedModelPath = modelPath;

			auto it = std::find(m_loadedModels.begin(), m_loadedModels.end(), modelPath);
			if (it == m_loadedModels.end()) {
				m_loadedModels.push_back(getFileName(modelPath));
				m_selectedModelIndex = m_loadedModels.size() - 1;

				InitializeTransformEditor(m_selectedModelIndex);
			}
			else {
				m_selectedModelIndex = std::distance(m_loadedModels.begin(), it);
			}
		}
	}

	void LeftPanelUI::InitializeTransformEditor(int modelIndex) {
		auto& editor = m_modelTransformEditors[modelIndex];
		editor.SetParent(shared_from_this());
	}

	void LeftPanelUI::RenderMaterialTab() {
		// ������ �� ���� (Box/Sphere) - ���� ��ư
		ImGui::Text("Preview Model:");
		if (ImGui::RadioButton("Box", &m_useBoxModel, 0)) {
			if (m_materialModelChangeCallback)
			{
				m_materialModelChangeCallback(0);
			}
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Sphere", &m_useBoxModel, 1)) {
			if (m_materialModelChangeCallback)
			{
				m_materialModelChangeCallback(1);
			}
		}

		ImGui::Separator();

		// PBR �ؽ�ó ���� UI
		ImGui::Text("Material Textures:");

		// Albedo �ؽ�ó (�ʼ�)
		DisplayTextureLoadUI("Albedo", m_albedoPath, [this](const std::string& path) { m_albedoPath = path; });

		// Normal �ؽ�ó (����)
		DisplayTextureLoadUI("Normal", m_normalPath, [this](const std::string& path) { m_normalPath = path; });

		// Metallic �ؽ�ó (����)
		DisplayTextureLoadUI("Metallic", m_metallicPath, [this](const std::string& path) { m_metallicPath = path; });

		// Roughness �ؽ�ó (����)
		DisplayTextureLoadUI("Roughness", m_roughnessPath, [this](const std::string& path) { m_roughnessPath = path; });

		// AO �ؽ�ó (����)
		DisplayTextureLoadUI("AO", m_aoPath, [this](const std::string& path) { m_aoPath = path; });

		ImGui::Separator();

		// ���� ���� ��ư
		bool canCreateMaterial = !m_albedoPath.empty(); // Albedo�� �ʼ�
		if (!canCreateMaterial) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Create Material", ImVec2(-1, 0))) {
			if (m_createMaterialCallback) {
				m_createMaterialCallback(
					m_materialName,
					m_albedoPath,
					m_normalPath,
					m_metallicPath,
					m_roughnessPath,
					m_aoPath
				);
			}
		}

		if (!canCreateMaterial) {
			ImGui::EndDisabled();
		}

		// ���� ���̾�α� ǥ�� �� ��� ó��
		HandleTextureFileBrowser();
	}

	void LeftPanelUI::DisplayTextureLoadUI(const char* labelName, std::string& texturePath, std::function<void(const std::string&)> onTextureSelected) {
		ImGui::Text("%s:", labelName);
		ImGui::SameLine(120); // ������ ��ġ�� �ؽ�ó �̸� ����

		std::string displayName = texturePath.empty() ? "None" : getFileName(texturePath);
		ImGui::Text("%s", displayName.c_str());
		ImGui::SameLine();

		// �ε� ��ư
		std::string buttonLabel = "Load##" + std::string(labelName);
		if (ImGui::Button(buttonLabel.c_str())) {
			m_currentSelectingTexturePath = texturePath;
			m_currentSelectingTextureCallback = onTextureSelected;

			// ���� ���̾�α� ����
			IGFD::FileDialogConfig config;
			if (!m_lastBrowsedDirectory.empty()) {
				config.path = m_lastBrowsedDirectory;
			}
			else {
				config.path = ".";
			}
			config.flags = ImGuiFileDialogFlags_Modal;

			std::string dialogId = m_fileDialogKeyMaterial + std::string(labelName);

			ImGuiFileDialog::Instance()->OpenDialog(
				dialogId,
				"Choose Texture",
				"Image files (*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr){.png,.jpg,.jpeg,.tga,.bmp,.hdr}",
				config);
		}

		// ���� ���õ� �ؽ�ó�� ������ Clear ��ư �߰�
		if (!texturePath.empty()) {
			ImGui::SameLine();
			std::string clearLabel = "Clear##" + std::string(labelName);
			if (ImGui::Button(clearLabel.c_str())) {
				texturePath.clear();
			}
		}
	}

	void LeftPanelUI::SetModelTransform(int modelIndex, const glm::mat4& mat) {
		SetModelTransform(modelIndex, ImGui::ModelTransform(mat));
	}

	void LeftPanelUI::SetModelTransform(int modelIndex, const ImGui::ModelTransform& transform) {
		if (modelIndex < 0 || modelIndex >= m_loadedModels.size())
			return;

		// �ش� ���� TransformEditor �ʱ�ȭ
		auto it = m_modelTransformEditors.find(modelIndex);
		if (it == m_modelTransformEditors.end()) {
			InitializeTransformEditor(modelIndex);

		}

		// Transform �� ����
		m_modelTransformEditors[modelIndex].SetTransform(transform);
	}

	std::string LeftPanelUI::GetTextureTypeName(TextureType type) {
		switch (type) {
		case TextureType::Albedo: return "Albedo";
		case TextureType::Normal: return "Normal";
		case TextureType::Metallic: return "Metallic";
		case TextureType::Roughness: return "Roughness";
		case TextureType::AO: return "AO";
		default: return "Unknown";
		}
	}

	void LeftPanelUI::OpenTextureFileBrowser(TextureType type) {
		// ���� ���õ� �ؽ�ó Ÿ�� ����
		m_currentSelectingTextureType = type;

		IGFD::FileDialogConfig config;
		// ������ ������ ���丮�� ������ �ش� ��ġ���� ����
		if (!m_lastBrowsedDirectory.empty()) {
			config.path = m_lastBrowsedDirectory;
		}
		else {
			config.path = ".";
		}
		config.flags = ImGuiFileDialogFlags_Modal;

		std::string dialogId = m_fileDialogKeyMaterial + std::to_string(static_cast<int>(type));

		ImGuiFileDialog::Instance()->OpenDialog(
			dialogId,
			"Choose Texture",
			"Image files (*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr){.png,.jpg,.jpeg,.tga,.bmp,.hdr}",
			config);
	}

	void LeftPanelUI::HandleTextureFileBrowser() {
		// �� �ǰ� ������ ũ�� ���� ���
		ImVec2 maxSize = ImGui::GetMainViewport()->Size;
		maxSize.x *= 0.4f;
		maxSize.y *= 0.4f;
		ImVec2 minSize = maxSize;

		// �� �ؽ�ó Ÿ�Կ� ���� ���̾�α� Ȯ��
		const char* textureTypes[] = { "Albedo", "Normal", "Metallic", "Roughness", "AO" };

		for (const char* type : textureTypes) {
			std::string dialogId = m_fileDialogKeyMaterial + std::string(type);

			if (ImGuiFileDialog::Instance()->Display(dialogId, ImGuiWindowFlags_NoCollapse, minSize, maxSize)) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
					std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
					m_lastBrowsedDirectory = filePath;
					m_currentSelectingTexturePath = filePathName;

					if (m_currentSelectingTextureCallback) {
						m_currentSelectingTextureCallback(filePathName);
					}
				}

				ImGuiFileDialog::Instance()->Close();
			}
		}
	}

	void LeftPanelUI::LoadCustomTexture(const std::string& path, TextureType type) {
		// ���� �̸� ����
		std::string fileName = getFileName(path);

		std::cout << "load texture " << path << std::endl;
	}

	RightPanelUI::RightPanelUI(VulkanTutorialExtension* extension) : m_extension(extension){
	}

	RightPanelUI::~RightPanelUI() = default;

	void RightPanelUI::Render(int x, int y, int width, int height, bool enabled) {
		if (!m_extension) {
			return; // VulkanTutorialExtension�� �������� �ʾ����� ���������� ����
		}

		// ������ ����
		ImGui::SetNextWindowPos(ImVec2(float(x), float(y)));
		ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse;

		if (!enabled) {
			window_flags |= ImGuiWindowFlags_NoInputs;
		}

		// �г� ���� (���� �ڵ��� â ���� ���)
		if (ImGui::Begin("##RightPanel", &m_open, window_flags)) {
			// Most "big" widgets share a common width settings by default.
			ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
			ImGui::Spacing();

			// ������ ������
			RenderContent();

			ImGui::PopItemWidth();
		}
		ImGui::End();
	}

	void RightPanelUI::RenderContent() {
		// ��� ������ �Լ� - ���Ʒ� ���м� �߰�
		auto renderHeaderWithLines = [](const char* label) {
			// ��� ���м� �߰�
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// ��� �ؽ�Ʈ ���� ����
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.95f, 1.0f, 1.0f)); // �� ���� ��� �迭

			// ��� ������
			bool isOpen = ImGui::CollapsingHeader(label);

			// �ؽ�Ʈ ���� ����
			ImGui::PopStyleColor();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			return isOpen;
			};

		// ��ü ����� ���� �⺻ ��Ÿ�� ���� (�� £�� ������ �Ķ��� �迭)
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.35f, 0.4f, 0.8f));         // �ణ �� ���� û�ϻ�
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.45f, 0.55f, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.35f, 0.5f, 0.65f, 0.8f));

		// Directional Light ����
		if (renderHeaderWithLines("Directional Light"), ImGuiTreeNodeFlags_DefaultOpen) {
			static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
			const ImGuiSliderFlags flags_for_sliders = flags & ~ImGuiSliderFlags_WrapAround;

			ImGui::Checkbox("Use Directional Light", &m_extension->useDirectionalLight);
			ImGui::SliderFloat("DirectionalLightIntensity", &m_extension->directionalLightIntensity, 0.0f, 100.0f, "%.1f", flags_for_sliders);
			ImGui::Spacing();
		}

		// Point Lights ����
		static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
		const ImGuiSliderFlags flags_for_sliders = flags & ~ImGuiSliderFlags_WrapAround;

		if (renderHeaderWithLines("Point Lights"), ImGuiTreeNodeFlags_DefaultOpen) {
			if (ImGui::BeginTable("pointLights", 4)) {
				for (int i = 0; i < NR_POINT_LIGHTS; i++) {
					std::stringstream ss;
					ss << "Point Light " << i;
					ImGui::TableNextColumn();
					ImGui::Checkbox(ss.str().c_str(), &m_extension->pointLightsSwitch[i]);
				}
				ImGui::EndTable();
			}

			ImGui::SliderFloat("pointLightlinear", &m_extension->pointLightlinear, 0.0f, 1.0f, "%.3f", flags_for_sliders);
			ImGui::SliderFloat("pointLightQuadratic", &m_extension->pointLightQuadratic, 0.0f, 1.0f, "%.3f", flags_for_sliders);
			ImGui::SliderFloat("pointLightIntensity", &m_extension->pointLightIntensity, 0.0f, 100.0f, "%.1f", flags_for_sliders);
			ImGui::Spacing();
		}

		// Texture Viewer
		RenderTextureViewer();

		if (renderHeaderWithLines("Light Components"), ImGuiTreeNodeFlags_DefaultOpen) {
			ImGui::RadioButton("None", &m_extension->debugDisplayTarget, 0); ImGui::SameLine();
			ImGui::RadioButton("Specular", &m_extension->debugDisplayTarget, 7); ImGui::SameLine();
			ImGui::RadioButton("Diffuse", &m_extension->debugDisplayTarget, 8);
			ImGui::Spacing();
		}

		// Post Processing ����
		if (renderHeaderWithLines("Post Processing"), ImGuiTreeNodeFlags_DefaultOpen) {
			ImGui::SliderFloat("Exposure", &m_extension->exposure, 0.0f, 10.0f, "%.01f", flags_for_sliders);
			ImGui::Spacing();
		}

		// �⺻ ��Ÿ�� ����
		ImGui::PopStyleColor(3);

		// ����� ���ڿ� ���
		if (!ImGui::stringToDebug.empty()) {
			ImGui::Text("%s", ImGui::stringToDebug.c_str());
			ImGui::Spacing();
		}
	}

	void RightPanelUI::RenderTextureViewer() {
		if (!m_textureViewer) {
			return;
		}

		// ��� ���м� �߰�
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// �ؽ�ó ��� ��� (���� ����)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.95f, 1.0f, 1.0f)); // �� ���� ��� �迭

		bool isOpen = ImGui::CollapsingHeader("Texture Viewer", ImGuiTreeNodeFlags_DefaultOpen);

		// ���� ����
		ImGui::PopStyleColor();

		// ����� �������� ���� �ϴ� ���м� �߰�
		if (!isOpen) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			return;
		}

		ImGui::Spacing();

		const auto& textures = m_textureViewer->getTextures();

		// ���õ� �ؽ�ó ����
		bool hasSelectedTexture = (m_selectedTextureIndex >= 0 && m_selectedTextureIndex < textures.size());
		bool isCubemap = hasSelectedTexture && textures[m_selectedTextureIndex].cubeMap != nullptr;
		bool hasImage = hasSelectedTexture && textures[m_selectedTextureIndex].image != nullptr;

		// MIP ���� ����
		static int mipLevel = 0;
		int maxMipLevels = isCubemap ? textures[m_selectedTextureIndex].cubeMap->imageViews.size() : 1;

		if (hasImage) {
			// ���� �̹����� MIP ���� �� �������� (���� �������� ����)
			// maxMipLevels = textures[m_selectedTextureIndex].image->mipLevels;
		}

		ImGui::BeginDisabled(maxMipLevels <= 1);
		if (ImGui::SliderInt("Mip Level", &mipLevel, 0, maxMipLevels - 1)) {
			if (m_textureViewer) {
				m_textureViewer->selectMipLevel(mipLevel);
			}
		}
		ImGui::EndDisabled();

		// Cubemap Face ����
		static const char* cubemapFaces[] = {
			"Right (+X)", "Left (-X)",
			"Up (+Y)", "Down (-Y)",
			"Forward (+Z)", "Rear (-Z)"
		};
		static int cubemapFace = 0;

		ImGui::BeginDisabled(!isCubemap);
		if (ImGui::Combo("Cubemap Face", &cubemapFace, cubemapFaces, IM_ARRAYSIZE(cubemapFaces))) {
			if (m_textureViewer) {
				m_textureViewer->selectCubeMapFace(cubemapFace);
			}
		}
		ImGui::EndDisabled();

		// �������
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 1.0f, 1.0f)); // ���� �Ķ��� �迭
		ImGui::Text("Available Textures:");
		ImGui::PopStyleColor();

		ImGui::BeginChild("TextureList", ImVec2(0, 150), true);

		for (int i = 0; i < textures.size(); i++) {
			const bool isSelected = (i == m_selectedTextureIndex);
			if (ImGui::Selectable(textures[i].name.c_str(), isSelected)) {
				m_selectedTextureIndex = i;
				m_textureViewer->selectTexture(i);

				// �� �ؽ�ó ���� �� MIP/Face ���� ���� �ʱ�ȭ
				if (textures[i].cubeMap) {
					cubemapFace = 0;
					m_textureViewer->selectCubeMapFace(cubemapFace);
				}
				mipLevel = 0;
				m_textureViewer->selectMipLevel(mipLevel);
			}

			// ���õ� �׸� ����
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		if (textures.empty()) {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "�ؽ�ó�� �����ϴ�");
		}

		ImGui::EndChild();

		// ���õ� �ؽ�ó�� ���� ���� ǥ��
		if (hasSelectedTexture) {
			const auto& selectedTexture = textures[m_selectedTextureIndex];
			ImGui::Separator();

			// ������� ��Ÿ��
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 1.0f, 1.0f));
			ImGui::Text("Selected Texture: %s", selectedTexture.name.c_str());
			ImGui::PopStyleColor();

			// �ؽ�ó Ÿ�� ǥ��
			const char* textureType = "Unknown";
			if (selectedTexture.cubeMap) textureType = "Cubemap";
			else if (selectedTexture.image) textureType = "2D Texture";

			ImGui::Text("Texture Type: %s", textureType);
		}

		ImGui::Spacing();
	}

	bool TransformEditor::Render(int selectedIndex, const char* label) {
		bool confirmed = false;

		if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
			
			ImGui::Text("Position");
			ImGui::InputFloat3("##Position", m_transform.position, "%.3f");

			if (ImGui::IsItemDeactivatedAfterEdit()) {
				confirmed = true;
			}

			// ȸ��
			ImGui::Text("Rotation (degrees)");
			ImGui::InputFloat3("##Rotation", m_transform.rotation, "%.3f");

			if (ImGui::IsItemDeactivatedAfterEdit()) {
				confirmed = true;
			}

			// ũ��
			ImGui::Text("Scale");
			ImGui::InputFloat3("##Scale", m_transform.scale, "%.3f");

			if (ImGui::IsItemDeactivatedAfterEdit()) {
				confirmed = true;
			}

			// ���� ��ư
			if (ImGui::Button("Reset Transform", ImVec2(-1, 0))) {
				Reset();
				confirmed = true;
			}

			// Ȯ���� ��������� ������ �ݹ� ȣ��
			if (confirmed) {
				if (auto sharedParent = m_parent.lock()) {
					sharedParent->getModelTransformChangeCallback()(selectedIndex, m_transform);
				}
			}
		}

		return confirmed;
	}

	// Transform �ʱ�ȭ
	void TransformEditor::Reset() {
		m_transform.position[0] = m_transform.position[1] = m_transform.position[2] = 0.0f;
		m_transform.rotation[0] = m_transform.rotation[1] = m_transform.rotation[2] = 0.0f;
		m_transform.scale[0] = m_transform.scale[1] = m_transform.scale[2] = 1.0f;
	}

	ModelTransform::ModelTransform() = default;

	ModelTransform::ModelTransform(glm::mat4 mat)
	{
		// ��ġ ���� (����� ������ �� ���)
		position[0] = mat[3][0];
		position[1] = mat[3][1];
		position[2] = mat[3][2];

		// ũ�� ���� (�� �⺻ ������ ����)
		scale[0] = glm::length(glm::vec3(mat[0]));
		scale[1] = glm::length(glm::vec3(mat[1]));
		scale[2] = glm::length(glm::vec3(mat[2]));

		// ȸ�� ��� ���� (������ ����)
		glm::mat3 rotMat(
			glm::vec3(mat[0]) / scale[0],
			glm::vec3(mat[1]) / scale[1],
			glm::vec3(mat[2]) / scale[2]
		);

		// ȸ�� ����� ���Ϸ� ������ ��ȯ
		glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(glm::quat(rotMat)));
		rotation[0] = eulerAngles.x;
		rotation[1] = eulerAngles.y;
		rotation[2] = eulerAngles.z;
	}

	glm::mat4 ModelTransform::matrix() const
	{
		glm::mat4 transformMatrix = glm::mat4(1.0f);

		transformMatrix = glm::translate(transformMatrix,
			glm::vec3(position[0], position[1], position[2]));

		transformMatrix = glm::rotate(transformMatrix,
			glm::radians(rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix,
			glm::radians(rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix,
			glm::radians(rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));

		transformMatrix = glm::scale(transformMatrix,
			glm::vec3(scale[0], scale[1], scale[2]));

		return transformMatrix;
	}
}