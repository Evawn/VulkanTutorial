#include "Application.h"

void Application::Run() {
	InitWindow();
	InitVulkan();
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

	VWrap::QueueFamilyIndices indices = m_physical_device->FindQueueFamilies();

	m_graphics_queue = VWrap::Queue::Create(m_device, indices.graphicsFamily.value());
	m_present_queue = VWrap::Queue::Create(m_device, indices.presentFamily.value());
	m_transfer_queue = VWrap::Queue::Create(m_device, indices.transferFamily.value());
	m_graphics_command_pool = VWrap::CommandPool::Create(m_device, m_graphics_queue);
	m_transfer_command_pool = VWrap::CommandPool::Create(m_device, m_transfer_queue);

	m_frame_controller = VWrap::FrameController::Create(m_device, m_surface, m_graphics_command_pool, m_present_queue, MAX_FRAMES_IN_FLIGHT);
	m_frame_controller->SetResizeCallback([this]() { Resize(); });

	VkSampleCountFlagBits sample_count = m_physical_device->GetMaxUsableSampleCount();

	m_render_pass = VWrap::RenderPass::Create(m_device, m_frame_controller->GetSwapchain()->getFormat(), sample_count);

	CreateColorResources(sample_count);
	CreateDepthResources(sample_count);
	CreateFramebuffers();

	m_mesh_rasterizer = MeshRasterizer::Create(
		m_device, 
		m_render_pass,
		m_graphics_command_pool,
		m_frame_controller->GetSwapchain()->getExtent(),
		MAX_FRAMES_IN_FLIGHT);
}

void Application::MainLoop() {
	while (!glfwWindowShouldClose(m_glfw_window.get()[0])) {
		glfwPollEvents();
		DrawFrame();
	}
	vkDeviceWaitIdle(m_device->GetHandle());
}

void Application::Cleanup() {
	glfwDestroyWindow(m_glfw_window.get()[0]);
	glfwTerminate();
}

void Application::DrawFrame() {
	// ACQUIRE FRAME ------------------------------------------------
	m_frame_controller->AcquireNext();
	uint32_t image_index = m_frame_controller->GetImageIndex();
	uint32_t frame_index = m_frame_controller->GetCurrentFrame();
	auto command_buffer = m_frame_controller->GetCurrentCommandBuffer();

	// BEGIN RENDER PASS - TODO: ABSTRACT ----------------------------------------
	std::shared_ptr<VWrap::Framebuffer> framebuffer = m_framebuffers[image_index];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(command_buffer->GetHandle(), &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer.");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.framebuffer = framebuffer->GetHandle();
	renderPassInfo.renderPass = m_render_pass->GetHandle();
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = framebuffer->GetExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(command_buffer->GetHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// RECORD COMMANDS ------------------------------------------------
	m_mesh_rasterizer->CmdDraw(command_buffer, frame_index);
	m_mesh_rasterizer->UpdateUniformBuffer(frame_index);

	// END RENDER PASS - TODO: ABSTRACT ------------------------------------------------
	vkCmdEndRenderPass(command_buffer->GetHandle());

	if (vkEndCommandBuffer(command_buffer->GetHandle()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end command buffer recording!");
	}

	// RENDER -----------------------------------
	m_frame_controller->Render();
}

void Application::Resize() {
	CreateColorResources(m_render_pass->GetSamples());
	CreateDepthResources(m_render_pass->GetSamples());
	CreateFramebuffers();
	m_mesh_rasterizer->Resize(m_frame_controller->GetSwapchain()->getExtent());
}

void Application::CreateFramebuffers() {
	m_framebuffers.resize(m_frame_controller->GetSwapchain()->getImageCount());
	for (uint32_t i = 0; i < m_frame_controller->GetSwapchain()->getImageCount(); i++){
		std::vector<std::shared_ptr<VWrap::ImageView>> attachments = {
			m_color_image_view,
			m_depth_image_view,
			m_frame_controller->GetImageViews()[i]
		};

		m_framebuffers[i] = VWrap::Framebuffer::Create2D(m_device, m_render_pass, attachments, m_frame_controller->GetSwapchain()->getExtent());
	}
}

void Application::CreateDepthResources(VkSampleCountFlagBits samples)
{
	VkFormat depthFormat = VWrap::Image::FindDepthFormat(m_device);

	VWrap::ImageCreateInfo info{};
	info.format = depthFormat;
	info.height = m_frame_controller->GetSwapchain()->getExtent().height;
	info.width = m_frame_controller->GetSwapchain()->getExtent().width;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	info.mip_levels = 1;
	info.samples = samples;

	auto im = VWrap::Image::Create(m_device, m_graphics_command_pool, info);
	m_depth_image_view = VWrap::ImageView::Create(m_device, im, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Application::CreateColorResources(VkSampleCountFlagBits samples) 
{
	VWrap::ImageCreateInfo info{};
	info.format = m_frame_controller->GetSwapchain()->getFormat();
	info.height = m_frame_controller->GetSwapchain()->getExtent().height;
	info.width = m_frame_controller->GetSwapchain()->getExtent().width;
	info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	info.mip_levels = 1;
	info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.samples = samples;

	auto im = VWrap::Image::Create(m_device, m_graphics_command_pool, info);
	m_color_image_view = VWrap::ImageView::Create(m_device, im, VK_IMAGE_ASPECT_COLOR_BIT);
}
