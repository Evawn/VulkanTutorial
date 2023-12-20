#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "tiny_obj_loader.h"

#include "VWrap/Instance.h"
#include "VWrap/Surface.h"
#include "VWrap/PhysicalDevice.h"
#include "VWrap/Device.h"
#include "VWrap/Swapchain.h"
#include "VWrap/ImageView.h"
#include "VWrap/Image.h"
#include "VWrap/RenderPass.h"
#include "VWrap/DescriptorSetLayout.h"
#include "VWrap/Framebuffer.h"
#include "VWrap/Pipeline.h"
#include "VWrap/CommandPool.h"
#include "VWrap/CommandBuffer.h"
#include "VWrap/Fence.h"
#include "VWrap/Semaphore.h"
#include "VWrap/Sampler.h"
#include "VWrap/Buffer.h"
#include "VWrap/DescriptorPool.h"
#include "VWrap/DescriptorSet.h"
#include "VWrap/Queue.h"
#include "VWrap/FrameController.h"

#include "MeshRasterizer.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>

// CONSTANTS -------------------------------------------------------------------------------------------------

/// <summary>
/// The width and height of the viewport. (In screen coordinates.)
/// </summary>
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

/// <summary>
/// The maximum number of frames that can be in flight at once.
/// </summary>
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

/// <summary>
/// Whether or not to enable validation layers. (Debugging only.)
/// </summary>
#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

/// <summary>
/// This class is the main application class. It contains all the vulkan objects
/// and the main loop. 
/// </summary>
class Application {

private:

	// VWRAP OBJECTS ---------------------------------------------------------------------------------------------
	// VULKAN BASE
	std::shared_ptr<VWrap::Instance> m_instance;
	std::shared_ptr<VWrap::PhysicalDevice> m_physical_device;
	std::shared_ptr<VWrap::Device> m_device;

	// WINDOW PRESENTATION
	std::shared_ptr<GLFWwindow*> m_glfw_window;
	std::shared_ptr<VWrap::Surface> m_surface;
	std::shared_ptr<VWrap::FrameController> m_frame_controller;

	// COMMANDS
	std::shared_ptr<VWrap::CommandPool> m_graphics_command_pool;
	std::shared_ptr<VWrap::CommandPool> m_transfer_command_pool;
	std::shared_ptr<VWrap::Queue> m_graphics_queue;
	std::shared_ptr<VWrap::Queue> m_present_queue;
	std::shared_ptr<VWrap::Queue> m_transfer_queue;

	// RENDER PASS
	std::shared_ptr<VWrap::RenderPass> m_render_pass;
	std::vector<std::shared_ptr<VWrap::Framebuffer>> m_framebuffers;
	std::shared_ptr<VWrap::ImageView> m_depth_image_view;
	std::shared_ptr<VWrap::ImageView> m_color_image_view;

	/// <summary>
	/// Contains and manages the resources needed to render a mesh with rasterization.
	/// </summary>
	std::shared_ptr<MeshRasterizer> m_mesh_rasterizer;


	// CLASS FUNCTIONS -------------------------------------------------------------------------------------------
public:

	/// <summary>
	/// Entrypoint to the application. Creates all resources and runs the main loop. Handles cleanup.
	/// </summary>
	void Run();

private:
	/// <summary>
	/// Callback function for when the window is resized. Notifies the frame controller to resize.
	/// </summary>
	static void glfw_FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	/// <summary>
	/// Initializes the window and sets references in GLFW to the app and resize callback.
	/// </summary>
	void InitWindow();

	/// <summary>
	/// Initializes the vulkan objects.
	/// </summary>
	void InitVulkan();

	/// <summary>
	/// Contains the main loop of the application, which polls for events and draws frames.
	/// </summary>
	void MainLoop();

	/// <summary>
	/// Cleans up any resources that need to be explicitly destroyed.
	/// </summary>
	void Cleanup();

	/// <summary>
	/// Called by the frame controller when the window is resized. Resizes all necessary resources.
	/// </summary>
	void Resize();

	/// <summary>
	/// Creates a framebuffer for each swapchain image.
	/// </summary>
	void CreateFramebuffers();

	/// <summary>
	/// Creates the depth image and image view.
	/// </summary>
	void CreateDepthResources(VkSampleCountFlagBits samples);

	void CreateColorResources(VkSampleCountFlagBits samples);

	/// <summary>
	/// Acquires the current image and frame, records the command buffer, and submits its to be rendered.
	/// </summary>
	void DrawFrame();
};