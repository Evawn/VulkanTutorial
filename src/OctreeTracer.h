#pragma once
#include "vulkan/vulkan.h"
#include "Buffer.h"
#include "Device.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "Allocator.h"
#include "Sampler.h"

#include "Camera.h"

#include "tiny_obj_loader.h"
#include <unordered_map>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class OctreeTracer
{
private:

	// RESOURCES ---------------------------------------------------------------------------------------------
	// DEVICE RESOURCES
	std::shared_ptr<VWrap::Device> m_device;
	std::shared_ptr<VWrap::CommandPool> m_graphics_pool;
	std::shared_ptr<VWrap::Allocator> m_allocator;

	// DESCRIPTORS
	std::shared_ptr<VWrap::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<VWrap::DescriptorPool> m_descriptor_pool;
	std::vector<std::shared_ptr<VWrap::DescriptorSet>> m_descriptor_sets;

	// PIPELINE
	std::shared_ptr<VWrap::Pipeline> m_pipeline;
	VkExtent2D m_extent;

	// BRICK TEXTURE
	std::shared_ptr<VWrap::Image> m_brick_texture;
	std::shared_ptr<VWrap::ImageView> m_brick_texture_view;
	std::shared_ptr<VWrap::Sampler> m_sampler;


	// CLASS FUNCTIONS ---------------------------------------------------------------------------------------


public:


	static std::shared_ptr<OctreeTracer> Create(std::shared_ptr<VWrap::Allocator> allocator, std::shared_ptr<VWrap::Device> device, std::shared_ptr<VWrap::RenderPass> render_pass, std::shared_ptr<VWrap::CommandPool> graphics_pool, VkExtent2D extent, uint32_t num_frames);

	void CreateDescriptors(int max_sets);

	void CreatePipeline(std::shared_ptr<VWrap::RenderPass> render_pass);

	/// <summary>
/// Creates one uniform buffer for each frame in flight and maps them to host memory.
/// </summary>
	void CreateUniformBuffers();

	/// <summary>
	/// Updates the descriptor sets with the uniform buffers and image sampler.
	/// </summary>
	void WriteDescriptors();


	/// <summary>
	/// Records commands to the command_buffer to draw the model using rasterization.
	/// </summary>
	/// <param name="command_buffer"> The command buffer to record to. </param>
	/// <param name="frame"> Which frame-in-flight's resources to use. </param>
	void CmdDraw(std::shared_ptr<VWrap::CommandBuffer> command_buffer, uint32_t frame, std::shared_ptr<Camera> camera);


	/// <summary>
	/// Updates the uniform buffer for the given frame using perpetually-mapped memory.
	/// </summary>
	/// <param name="frame"> The index of the buffer to update. </param>
	void UpdateUniformBuffer(uint32_t frame, std::shared_ptr<Camera> camera);

	/// <summary>
	/// Updates the extent of the pipeline.
	/// </summary>
	/// <param name="extent"> The new extent. </param>
	void Resize(VkExtent2D extent) {
		m_extent = extent;
	}
};

