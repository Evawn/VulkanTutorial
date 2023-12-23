#include "Application.h"

void Application::Run() {
	InitWindow();
	InitVulkan();
	InitImGui();
	MainLoop();
	Cleanup();
}

void Application::InitWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_glfw_window = std::make_shared<GLFWwindow*>(glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr));

	glfwSetWindowUserPointer(m_glfw_window.get()[0], this);
	glfwSetFramebufferSizeCallback(m_glfw_window.get()[0], glfw_FramebufferResizeCallback);
}

void Application::glfw_FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->m_frame_controller->SetResized(true);
}

void Application::InitVulkan() {
	m_instance = VWrap::Instance::Create(ENABLE_VALIDATION_LAYERS);
	m_surface = VWrap::Surface::Create(m_instance, m_glfw_window);
	m_physical_device = VWrap::PhysicalDevice::Pick(m_instance, m_surface);
	m_device = VWrap::Device::Create(m_physical_device, ENABLE_VALIDATION_LAYERS);
	m_allocator = VWrap::Allocator::Create(m_instance, m_physical_device, m_device);

	VWrap::QueueFamilyIndices indices = m_physical_device->FindQueueFamilies();

	m_graphics_queue = VWrap::Queue::Create(m_device, indices.graphicsFamily.value());
	m_present_queue = VWrap::Queue::Create(m_device, indices.presentFamily.value());
	m_transfer_queue = VWrap::Queue::Create(m_device, indices.transferFamily.value());
	m_graphics_command_pool = VWrap::CommandPool::Create(m_device, m_graphics_queue);
	m_transfer_command_pool = VWrap::CommandPool::Create(m_device, m_transfer_queue);

	m_frame_controller = VWrap::FrameController::Create(m_device, m_surface, m_graphics_command_pool, m_present_queue, MAX_FRAMES_IN_FLIGHT);
	m_frame_controller->SetResizeCallback([this]() { Resize(); });

	VkSampleCountFlagBits sample_count = m_physical_device->GetMaxUsableSampleCount();

	m_render_pass = VWrap::RenderPass::CreateImGUI(m_device, m_frame_controller->GetSwapchain()->GetFormat(), sample_count);

	CreateColorResources(sample_count);
	CreateDepthResources(sample_count);
	CreateFramebuffers();

	m_mesh_rasterizer = MeshRasterizer::Create(
		m_allocator,
		m_device, 
		m_render_pass,
		m_graphics_command_pool,
		m_frame_controller->GetSwapchain()->GetExtent(),
		MAX_FRAMES_IN_FLIGHT);
}

void Application::InitImGui() {
	m_gui_renderer = GUIRenderer::Create(m_device);

	VWrap::QueueFamilyIndices indices = m_physical_device->FindQueueFamilies();
	ImGui_ImplGlfw_InitForVulkan(m_glfw_window.get()[0], true);

	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = m_instance->Get();
	init_info.PhysicalDevice = m_physical_device->Get();
	init_info.Device = m_device->Get();
	init_info.QueueFamily = indices.graphicsFamily.value();
	init_info.Queue = m_graphics_queue->Get();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_gui_renderer->GetDescriptorPool()->Get();
	init_info.Subpass = 1;
	init_info.MinImageCount = m_frame_controller->GetSwapchain()->Size();
	init_info.ImageCount = m_frame_controller->GetSwapchain()->Size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, m_render_pass->Get());
}

void Application::MainLoop() {
	while (!glfwWindowShouldClose(m_glfw_window.get()[0])) {
		glfwPollEvents();
		//DrawFrame();
		DrawFrame();
	}
	vkDeviceWaitIdle(m_device->Get());
}

void Application::Cleanup() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_glfw_window.get()[0]);
	glfwTerminate();
}

void Application::DrawFrame() {

	// ACQUIRE FRAME ------------------------------------------------
	m_frame_controller->AcquireNext();
	uint32_t image_index = m_frame_controller->GetImageIndex();
	uint32_t frame_index = m_frame_controller->GetCurrentFrame();
	auto command_buffer = m_frame_controller->GetCurrentCommandBuffer();

	// BEGIN RENDER PASS ------------------------------------------------
	std::shared_ptr<VWrap::Framebuffer> framebuffer = m_framebuffers[image_index];
	VWrap::CommandBuffer::Begin(command_buffer);
	VWrap::CommandBuffer::CmdBeginRenderPass(command_buffer, m_render_pass, framebuffer);

	// RECORD SCENE COMMANDS ------------------------------------------------
	m_mesh_rasterizer->CmdDraw(command_buffer, frame_index);
	m_mesh_rasterizer->UpdateUniformBuffer(frame_index);

	// NEXT SUBPASS ------------------------------------------------
	vkCmdNextSubpass(command_buffer->Get(), VK_SUBPASS_CONTENTS_INLINE);

	// RECORD GUI COMMANDS ------------------------------------------------
	m_gui_renderer->CmdDraw(command_buffer);

	// END RENDER PASS - TODO: ABSTRACT ------------------------------------------------
	vkCmdEndRenderPass(command_buffer->Get());

	if (vkEndCommandBuffer(command_buffer->Get()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end command buffer recording!");
	}

	// RENDER -----------------------------------
	m_frame_controller->Render();
}

void Application::Resize() {
	CreateColorResources(m_render_pass->GetSamples());
	CreateDepthResources(m_render_pass->GetSamples());
	CreateFramebuffers();
	m_mesh_rasterizer->Resize(m_frame_controller->GetSwapchain()->GetExtent());
}

void Application::CreateFramebuffers() {
	m_framebuffers.resize(m_frame_controller->GetSwapchain()->Size());
	for (uint32_t i = 0; i < m_frame_controller->GetSwapchain()->Size(); i++){
		std::vector<std::shared_ptr<VWrap::ImageView>> attachments = {
			m_color_image_view,
			m_depth_image_view,
			m_frame_controller->GetImageViews()[i]
		};

		m_framebuffers[i] = VWrap::Framebuffer::Create2D(m_device, m_render_pass, attachments, m_frame_controller->GetSwapchain()->GetExtent());
	}
}

void Application::CreateDepthResources(VkSampleCountFlagBits samples)
{
	VkFormat depthFormat = VWrap::FindDepthFormat(m_physical_device->Get());

	VWrap::ImageCreateInfo info{};
	info.format = depthFormat;
	info.height = m_frame_controller->GetSwapchain()->GetExtent().height;
	info.width = m_frame_controller->GetSwapchain()->GetExtent().width;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	info.mip_levels = 1;
	info.samples = samples;

	auto im = VWrap::Image::Create(m_allocator, info);
	m_depth_image_view = VWrap::ImageView::Create(m_device, im, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Application::CreateColorResources(VkSampleCountFlagBits samples) 
{
	VWrap::ImageCreateInfo info{};
	info.format = m_frame_controller->GetSwapchain()->GetFormat();
	info.height = m_frame_controller->GetSwapchain()->GetExtent().height;
	info.width = m_frame_controller->GetSwapchain()->GetExtent().width;
	info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	info.mip_levels = 1;
	info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.samples = samples;

	auto im = VWrap::Image::Create(m_allocator, info);
	m_color_image_view = VWrap::ImageView::Create(m_device, im, VK_IMAGE_ASPECT_COLOR_BIT);
}
