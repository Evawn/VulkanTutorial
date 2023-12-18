#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<VWrap::Vertex> {
		size_t operator()(VWrap::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}


#include <chrono>

// CONSTANTS -------------------------------------------------------------------------------------------------

/// <summary>
/// The width and height of the viewport. (In screen coordinates.)
/// </summary>
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};



/// <summary>
/// This class is the main application class. It contains all the vulkan objects
/// and the main loop. 
/// </summary>
class HelloTriangleApplication {

public:
	/// <summary> Whether or not the window has been resized. </summary>
	bool resized = false;

private:

	// CLASS MEMBERS --------------------------------------------------------------------------------------------

	/// <summary> The maximum number of frames in flight. </summary>
	uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	/// <summary> The current frame. </summary>
	uint32_t currentFrame = 0;

private:

	// STRUCTS
	/// <summary> The indices of the index buffer. </summary>
	//const std::vector<uint16_t> indices = {
	//0, 1, 2, 2, 3, 0,
	//4, 5, 6, 6, 7, 4
	//};
	std::vector<uint32_t> indices;

	/// <summary> The vertices of the vertex buffer. </summary>
	//const std::vector<VWrap::Vertex> vertices = {
	//{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	//};
	std::vector<VWrap::Vertex> vertices;

	// VWRAP OBJECTS ---------------------------------------------------------------------------------------------

	std::shared_ptr<VWrap::Instance> m_instance;
	std::shared_ptr<VWrap::Surface> m_surface;
	std::shared_ptr<VWrap::PhysicalDevice> m_physical_device;
	std::shared_ptr<VWrap::Device> m_device;

	std::shared_ptr<VWrap::Swapchain> m_swapchain;
	std::vector<std::shared_ptr<VWrap::ImageView>> m_swapchain_image_views;

	std::shared_ptr<VWrap::ImageView> m_texture_image_view;
	std::shared_ptr<VWrap::Image> m_texture_image;

	std::shared_ptr<VWrap::RenderPass> m_render_pass;
	std::shared_ptr<VWrap::DescriptorSetLayout> m_descriptor_set_layout;
	std::vector<std::shared_ptr<VWrap::Framebuffer>> m_framebuffers;
	std::shared_ptr<VWrap::Pipeline> m_pipeline;

	std::shared_ptr<VWrap::CommandPool> m_graphics_command_pool;
	std::shared_ptr<VWrap::CommandPool> m_transfer_command_pool;

	std::vector<std::shared_ptr<VWrap::CommandBuffer>> m_render_command_buffers;
	std::vector<std::shared_ptr<VWrap::Semaphore>> m_image_available_semaphores;
	std::vector<std::shared_ptr<VWrap::Semaphore>> m_render_finished_semaphores;
	std::vector<std::shared_ptr<VWrap::Fence>> m_in_flight_fences;

	std::shared_ptr<VWrap::Sampler> m_sampler;
	std::shared_ptr<VWrap::Buffer> m_vertex_buffer;
	std::shared_ptr<VWrap::Buffer> m_index_buffer;
	std::vector<std::shared_ptr<VWrap::Buffer>> m_uniform_buffers;
	std::vector<void*> m_uniform_buffers_mapped;

	std::shared_ptr<VWrap::DescriptorPool> m_descriptor_pool;
	std::vector<std::shared_ptr<VWrap::DescriptorSet>> m_descriptor_sets;

	std::shared_ptr<VWrap::Queue> m_graphics_queue;
	std::shared_ptr<VWrap::Queue> m_present_queue;
	std::shared_ptr<VWrap::Queue> m_transfer_queue;

	std::shared_ptr<VWrap::Image> m_depth_image;
	std::shared_ptr<VWrap::ImageView> m_depth_image_view;

	std::shared_ptr<GLFWwindow*> m_glfw_window;


	// CLASS FUNCTIONS -------------------------------------------------------------------------------------------
public:

	/// <summary>
	/// Entrypoint to the application. Creates all resources and runs the main loop. Handles cleanup.
	/// </summary>
	void run();

private:
	/// <summary>
	/// Callback function for when the window is resized. Sets the resized flag to true.
	/// </summary>
	/// <param name="window"> The window handle. </param>
	/// <param name="width"> The new width of the window. </param>
	/// <param name="height"> The new height of the window. </param>
	static void glfw_FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	/// <summary>
	/// Initializes the window and sets references in GLFW to the app and resize callback.
	/// </summary>
	void initWindow();

	/// <summary>
	/// Initializes the vulkan objects.
	/// </summary>
	void initVulkan();

	/// <summary>
	/// Contains the main loop of the application, which polls for events and draws frames.
	/// </summary>
	void mainLoop();

	/// <summary>
	/// Cleans up any resources that need to be explicitly destroyed.
	/// </summary>
	void cleanup();

	/// <summary>
	/// Recreates the swapchain and all dependent resources.
	/// </summary>
	void recreateSwapchain();

	/// <summary>
	/// Creates the image views corresponding to the swapchain images.
	/// </summary>
	void CreateSwapchainImageViews();

	/// <summary>
	/// Creates a framebuffer for each swapchain image.
	/// </summary>
	void CreateFramebuffers();

	/// <summary>
	/// Creates synchronization objects needed for each frame in flight.
	/// </summary>
	void CreateRenderSyncObjects();

	/// <summary>
	/// Creates the command buffers used for rendering, one for each frmae in flight.
	/// </summary>
	void CreateRenderCommandBuffers();

	/// <summary>
	/// Creates the vertex buffer and copies the vertex data into it.
	/// </summary>
	void CreateVertexBuffer();

	/// <summary>
	/// Creates the index buffer and copies the index data into it.
	/// </summary>
	void CreateIndexBuffer();

	/// <summary>
	/// Creates one uniform buffer for each frame in flight and maps them to host memory.
	/// </summary>
	void CreateUniformBuffers();

	/// <summary>
	/// Updates the descriptor sets with the uniform buffers and image sampler.
	/// </summary>
	void UpdateDescriptorSets();

	/// <summary>
	/// Creates the depth image and image view.
	/// </summary>
	void CreateDepthResources();

	void LoadModel();

	/// <summary>
	/// Acquires image from the swapchain, records the command buffer, and submits it to the graphics queue.
	/// Then presents the image to the screen.
	/// </summary>
	void drawFrame();

	/// <summary>
	/// Updates the uniform buffer for the given frame using perpetually-mapped memory.
	/// </summary>
	/// <param name="frame"> The index of the buffer to update. </param>
	void updateUniformBuffer(uint32_t frame);

	/// <summary>
	/// Records the given command buffer to draw to the given framebuffer.
	/// </summary>
	/// <param name="commandBuffer"> The command buffer to record to. </param>
	/// <param name="imageIndex"> The index of the swapchain image to draw to. </param>
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

#endif