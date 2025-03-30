#include <algorithm>
#include "VulkanTutorialExtensionImGui.h"
#include "VulkanTutorialExtension.h"
#include "UniformBufferTypes.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuiFileDialog.h"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
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
	init_info.Device = device;
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
	// ImGui DescriptorPool은 한번만 만든다.
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
	// ImGui 내부에서 Pool에서 할당된 DescriptorSet 각각을 메모리 해제한다. 그래서 이 플래그가 필요하다. 
	// 내부적으로 Pool 해제도 같이 해준다.
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &imGuiDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanTutorialExtension::drawImGui(uint32_t imageIndex)
{
	// 창 크기 계산
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float windowWidth = viewport->Size.x;
	const float windowHeight = viewport->Size.y;
	const float PanelWidth = windowWidth * 0.15f;
	const float rightPanelX = windowWidth - PanelWidth;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();

	// 왼쪽에는 모델과 머티리얼 탭이 있는 패널
	static LeftPanelUI leftPanel;
	static bool leftPanelInitialized = false;

	if (!leftPanelInitialized) {
		leftPanel.SetModelLoadCallback([this](const std::string& modelPath) {
			// 파일 다이얼로그에서 이미 파일 경로를 받아옴
			bool success = LoadGLTFModel(modelPath);
			leftPanel.SetModelLoadResult(success, modelPath);
			});
		leftPanelInitialized = true;
	}
	leftPanel.Render(0, 0, PanelWidth, windowHeight);

	// 오른쪽에는 기존 Vulkan Tutorial Extension 창
	static RightPanelUI rightPanel;
	static bool initialized = false;

	if (!initialized) {
		rightPanel.SetVulkanExtension(this);
		initialized = true;
	}

	// 오른쪽 패널 렌더링
	rightPanel.Render(rightPanelX, 0, PanelWidth, windowHeight);

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

	if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &imGuiCommandPool) != VK_SUCCESS) {
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
	if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, imGuiCommandBuffers.data()) != VK_SUCCESS)
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
		VkResult err = vkCreateFramebuffer(device, &info, nullptr, &imGuiFrameBuffers[i]);
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
	// 이렇게 하려면 resolve attachment의 finalLayout을 present가 아닌 값으로 수정해야되지 않을까?
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
	if (vkCreateRenderPass(device, &info, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Could not create Dear ImGui's render pass");
	}
}

void VulkanTutorialExtension::cleanUpImGuiSwapchain()
{
	// Resources to destroy on swapchain recreation
	for (auto framebuffer : imGuiFrameBuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(device, imGuiRenderPass, nullptr);
}

void VulkanTutorialExtension::cleanUpImGui()
{
	// Resources to destroy when the program ends
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(device, imGuiDescriptorPool, nullptr);
	vkFreeCommandBuffers(device, imGuiCommandPool, static_cast<uint32_t>(imGuiCommandBuffers.size()), imGuiCommandBuffers.data());
	vkDestroyCommandPool(device, imGuiCommandPool, nullptr);
}

LeftPanelUI::LeftPanelUI() {
	// 초기화 코드
}

LeftPanelUI::~LeftPanelUI() {
	// 정리 코드
}

void LeftPanelUI::Render(int x, int y, int width, int height, bool enabled) {
	// 윈도우 설정
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

	// 패널 시작
	ImGui::Begin("LeftPanel", nullptr, window_flags);

	// 탭 바 생성
	if (ImGui::BeginTabBar("LeftPanelTabs")) {
		// 모델 탭
		if (ImGui::BeginTabItem("Model")) {
			m_state.prevSelectedTab = m_state.selectedTab;
			m_state.selectedTab = LeftPanelState::ModelTab;

			RenderModelTab();

			ImGui::EndTabItem();
		}

		// 재질 탭
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

void LeftPanelUI::RenderModelTab() {
	if (ImGui::CollapsingHeader("Load Model", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Load a 3D model in GLTF format");

		if (m_state.modelLoaded && !m_state.lastLoadedModelPath.empty()) {
			ImGui::TextWrapped("Current model: %s", m_state.lastLoadedModelPath.c_str());
		}

		if (ImGui::Button("Browse GLTF Model...", ImVec2(-1, 0))) {
			ImGuiFileDialog::Instance()->OpenDialog(
				m_fileDialogKey,
				"Choose GLTF Model",
				".gltf,.glb",
				"."
			);
		}

		// 파일 다이얼로그 표시 및 결과 처리
		if (ImGuiFileDialog::Instance()->Display(m_fileDialogKey)) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

				if (m_modelLoadCallback) {
					m_modelLoadCallback(filePathName);
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}

	if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (m_loadedModels.empty()) {
			ImGui::Text("No models loaded yet.");
		}
		else {
			ImGui::BeginChild("ModelsList", ImVec2(0, 200), true);
			for (int i = 0; i < m_loadedModels.size(); i++) {
				bool isSelected = (m_selectedModelIndex == i);
				if (ImGui::Selectable(m_loadedModels[i].c_str(), &isSelected)) {
					m_selectedModelIndex = i;
					if (m_modelLoadCallback) {
						m_modelLoadCallback(m_loadedModels[i]);
					}
				}
			}
			ImGui::EndChild();
		}
	}

	if (m_selectedModelIndex >= 0 && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		static float position[3] = { 0.0f, 0.0f, 0.0f };
		static float rotation[3] = { 0.0f, 0.0f, 0.0f };
		static float scale[3] = { 1.0f, 1.0f, 1.0f };

		ImGui::Text("Position");
		ImGui::DragFloat3("##Position", position, 0.1f);

		ImGui::Text("Rotation (degrees)");
		ImGui::DragFloat3("##Rotation", rotation, 1.0f);

		ImGui::Text("Scale");
		ImGui::DragFloat3("##Scale", scale, 0.1f);

		if (ImGui::Button("Reset Transform", ImVec2(-1, 0))) {
			position[0] = position[1] = position[2] = 0.0f;
			rotation[0] = rotation[1] = rotation[2] = 0.0f;
			scale[0] = scale[1] = scale[2] = 1.0f;
		}
	}
}

void LeftPanelUI::SetModelLoadResult(bool success, const std::string& modelPath) {
	if (success) {
		m_state.modelLoaded = true;
		m_state.lastLoadedModelPath = modelPath;

		auto it = std::find(m_loadedModels.begin(), m_loadedModels.end(), modelPath);
		if (it == m_loadedModels.end()) {
			m_loadedModels.push_back(modelPath);
			m_selectedModelIndex = m_loadedModels.size() - 1;
		}
		else {
			m_selectedModelIndex = std::distance(m_loadedModels.begin(), it);
		}
	}
}

void LeftPanelUI::RenderMaterialTab() {
	// 재질 탭의 UI 내용을 여기에 구현
	// 예시:
	ImGui::Text("Material tab content goes here");

	// 스크롤 영역 구현 예시
	ImGui::BeginChild("MaterialScrollArea", ImVec2(0, 300), true);
	for (int i = 0; i < 10; i++) {
		ImGui::Text("Material property %d", i);
	}
	ImGui::EndChild();
}

RightPanelUI::RightPanelUI() {
	// 초기화 코드
}

RightPanelUI::~RightPanelUI() {
	// 정리 코드
}

void RightPanelUI::Render(int x, int y, int width, int height, bool enabled) {
	if (!m_extension) {
		return; // VulkanTutorialExtension이 설정되지 않았으면 렌더링하지 않음
	}

	// 윈도우 설정
	ImGui::SetNextWindowPos(ImVec2(float(x), float(y)));
	ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse;

	if (!enabled) {
		window_flags |= ImGuiWindowFlags_NoInputs;
	}

	// 패널 시작 (기존 코드의 창 구조 사용)
	if (ImGui::Begin("##RightPanel", &m_open, window_flags)) {
		// Most "big" widgets share a common width settings by default.
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		ImGui::Spacing();

		// 컨텐츠 렌더링
		RenderContent();

		ImGui::PopItemWidth();
	}
	ImGui::End();
}

void RightPanelUI::RenderContent() {
	// 기존 코드에 있던 내용을 여기로 옮김

	// Instancing 섹션
	if (ImGui::CollapsingHeader("Instancing")) {
		ImGui::InputInt("Instance Count", &m_extension->instanceCount);
		m_extension->instanceCount = std::clamp(m_extension->instanceCount, 1, m_extension->maxInstanceCount);
	}

	// Directional Light 섹션
	if (ImGui::CollapsingHeader("Directional Light")) {
		static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
		const ImGuiSliderFlags flags_for_sliders = flags & ~ImGuiSliderFlags_WrapAround;

		ImGui::Checkbox("Use Directional Light", &m_extension->useDirectionalLight);
		ImGui::SliderFloat("DirectionalLightIntensity", &m_extension->directionalLightIntensity, 0.0f, 100.0f, "%.1f", flags_for_sliders);
	}

	// Point Lights 섹션
	static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
	const ImGuiSliderFlags flags_for_sliders = flags & ~ImGuiSliderFlags_WrapAround;

	if (ImGui::CollapsingHeader("Point Lights")) {
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
	}

	// DebugGBuffers 섹션
	if (ImGui::CollapsingHeader("DebugGBuffers")) {
		ImGui::RadioButton("None", &m_extension->debugDisplayTarget, 0); ImGui::SameLine();
		ImGui::RadioButton("Position", &m_extension->debugDisplayTarget, 1); ImGui::SameLine();
		ImGui::RadioButton("Normal", &m_extension->debugDisplayTarget, 2); ImGui::SameLine();
		ImGui::RadioButton("Albedo", &m_extension->debugDisplayTarget, 3);
		ImGui::RadioButton("AO", &m_extension->debugDisplayTarget, 4); ImGui::SameLine();
		ImGui::RadioButton("Roughness", &m_extension->debugDisplayTarget, 5); ImGui::SameLine();
		ImGui::RadioButton("Metallic", &m_extension->debugDisplayTarget, 6);
	}

	// Light Components 섹션
	if (ImGui::CollapsingHeader("Light Components")) {
		ImGui::RadioButton("NoneNone", &m_extension->debugDisplayTarget, 0); ImGui::SameLine();
		ImGui::RadioButton("Specular", &m_extension->debugDisplayTarget, 7); ImGui::SameLine();
		ImGui::RadioButton("Diffuse", &m_extension->debugDisplayTarget, 8);
	}

	// Post Processing 섹션
	if (ImGui::CollapsingHeader("Post Processing")) {
		ImGui::SliderFloat("Exposure", &m_extension->exposure, 0.0f, 10.0f, "%.01f", flags_for_sliders);
	}

	ImGui::Spacing();

	// 디버그 문자열 출력
	if (!ImGui::stringToDebug.empty()) {
		ImGui::Text("%s", ImGui::stringToDebug.c_str());
		ImGui::Spacing();
	}
}