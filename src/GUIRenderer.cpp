#include "GUIRenderer.h"

std::shared_ptr<GUIRenderer> GUIRenderer::Create(std::shared_ptr<VWrap::Device> device) {
	auto ret = std::make_shared<GUIRenderer>();

	// Setup Dear ImGui context
	std::vector<VkDescriptorPoolSize> pool_sizes =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
	};
	ret->m_imgui_descriptor_pool = VWrap::DescriptorPool::Create(device, pool_sizes, 1, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	return ret;
}

void GUIRenderer::CmdDraw(std::shared_ptr<VWrap::CommandBuffer> command_buffer, float time, float fps) {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();
	// Variables to manage simulation state and render time
	bool isSimulationPaused = false;

	// Create a window
	ImGui::Begin("Simulation Control");

	// Display the render time
	ImGui::Text("Render Time: %.3f ms", time);
	ImGui::Text("FPS: %.3f ms", fps);

	// Button to pause the simulation
	if (ImGui::Button("Pause")) {
		isSimulationPaused = true;
		// Additional code to pause the simulation
	}

	// Small space between buttons
	ImGui::SameLine();

	// Button to resume the simulation
	if (ImGui::Button("Resume")) {
		isSimulationPaused = false;
		// Additional code to resume the simulation
	}

	// End the ImGUI window
	ImGui::End();
	ImGui::EndFrame();

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer->Get());
}
