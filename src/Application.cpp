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

	float dpi_scale;
	glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &dpi_scale, nullptr);
	m_glfw_window = std::make_shared<GLFWwindow*>(glfwCreateWindow(WIDTH*dpi_scale, HEIGHT*dpi_scale, "Vulkan", nullptr, nullptr));

	glfwSetWindowUserPointer(m_glfw_window.get()[0], this);
	glfwSetFramebufferSizeCallback(m_glfw_window.get()[0], glfw_FramebufferResizeCallback);
	glfwSetKeyCallback(m_glfw_window.get()[0], glfw_KeyCallback);
}

void Application::glfw_FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->m_frame_controller->SetResized(true);
}

void Application::glfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	switch(key){
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	default:
		return;
	}
}

void Application::PollMoveState(GLFWwindow* window) {
	move_state.up = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	move_state.down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	move_state.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
	move_state.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
	move_state.forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
	move_state.back = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
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

	VkExtent2D extent = m_frame_controller->GetSwapchain()->GetExtent();
	m_mesh_rasterizer = MeshRasterizer::Create(
		m_allocator,
		m_device, 
		m_render_pass,
		m_graphics_command_pool,
		extent,
		MAX_FRAMES_IN_FLIGHT);

	m_gpu_profiler = GPUProfiler::Create(m_device, MAX_FRAMES_IN_FLIGHT);

	m_camera = Camera::Create(45, ((float)extent.width/(float)extent.height), 0.1f, 10.0f);
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

	float dpi_scale;
	glfwGetWindowContentScale(m_glfw_window.get()[0], &dpi_scale, nullptr);
	m_gui_renderer->SetDpiScale(dpi_scale);
}

void Application::MainLoop() {
	auto last_time = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(m_glfw_window.get()[0])) {
		auto current_time = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count();
		last_time = current_time;

		move_state = { false, false, false, false, false, false };
		glfwPollEvents();
		PollMoveState(m_glfw_window.get()[0]);
		MoveCamera(dt);
		//if (move_state.up) std::cout << "UP ";
		//if (move_state.down) std::cout << "DOWN ";
		//if (move_state.left) std::cout << "LEFT ";
		//if (move_state.right) std::cout << "RIGHT ";
		//if (move_state.forward) std::cout << "FORWARD ";
		//if (move_state.back) std::cout << "BACK ";
		//std::cout << std::endl;
		
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

	// BEGIN RECORDING ------------------------------------------------
	std::shared_ptr<VWrap::Framebuffer> framebuffer = m_framebuffers[image_index];
	VWrap::CommandBuffer::Begin(command_buffer);

	// BEGIN PROFILING ------------------------------------------------
	m_gpu_profiler->CmdBegin(command_buffer, frame_index);

	// BEGIN RENDER PASS ------------------------------------------------
	VWrap::CommandBuffer::CmdBeginRenderPass(command_buffer, m_render_pass, framebuffer);

	// RECORD SCENE COMMANDS ------------------------------------------------
	m_mesh_rasterizer->UpdateUniformBuffer(frame_index, m_camera);
	m_mesh_rasterizer->CmdDraw(command_buffer, frame_index);
	

	// END PROFILING ------------------------------------------------
	m_gpu_profiler->CmdEnd(command_buffer, frame_index);

	// NEXT SUBPASS ------------------------------------------------
	vkCmdNextSubpass(command_buffer->Get(), VK_SUBPASS_CONTENTS_INLINE);

	// RECORD GUI COMMANDS ------------------------------------------------
	GPUProfiler::PerformanceMetrics metrics = m_gpu_profiler->GetMetrics(frame_index);
	m_gui_renderer->CmdDraw(command_buffer, metrics.render_time, metrics.fps);

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

	VkExtent2D extent = m_frame_controller->GetSwapchain()->GetExtent();
	m_mesh_rasterizer->Resize(extent);
	m_camera = Camera::Create(45, ((float)extent.width / (float)extent.height), 0.1f, 10.0f);

	float dpi_scale;
	glfwGetWindowContentScale(m_glfw_window.get()[0], &dpi_scale, nullptr);
	m_gui_renderer->SetDpiScale(dpi_scale);
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

void Application::MoveCamera(float dt) {
	float speed = 2.0f;
	float distance = speed * dt;

	if (move_state.up && !move_state.down) m_camera->MoveUp(distance);
	if (move_state.down && !move_state.up) m_camera->MoveUp(-distance);
	if (move_state.left && !move_state.right) m_camera->MoveRight(-distance);
	if (move_state.right && !move_state.left) m_camera->MoveRight(distance);
	if (move_state.forward && !move_state.back) m_camera->MoveForward(distance);
	if (move_state.back && !move_state.forward) m_camera->MoveForward(-distance);
}
