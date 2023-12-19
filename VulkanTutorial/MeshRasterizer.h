#pragma once
#include "vulkan/vulkan.h"
#include "VWrap/Buffer.h"
#include "VWrap/Device.h"
#include "VWrap/CommandPool.h"
#include "VWrap/CommandBuffer.h"
#include "VWrap/DescriptorSet.h"
#include "VWrap/DescriptorSetLayout.h"
#include "VWrap/DescriptorPool.h"
#include "VWrap/RenderPass.h"
#include "VWrap/Framebuffer.h"
#include "VWrap/Pipeline.h"
#include "VWrap/Image.h"
#include "VWrap/ImageView.h"
#include "VWrap/Sampler.h"

#include "tiny_obj_loader.h"

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


#include <unordered_map>
#include <chrono>

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};


class MeshRasterizer
{
private:

	// STRUCTS
/// <summary> The indices of the index buffer. </summary>
	std::vector<uint32_t> indices;

	/// <summary> The vertices of the vertex buffer. </summary>
	std::vector<VWrap::Vertex> vertices;

	std::shared_ptr<VWrap::Device> m_device;
	std::shared_ptr<VWrap::ImageView> m_texture_image_view;
	std::shared_ptr<VWrap::Image> m_texture_image;
	std::shared_ptr<VWrap::DescriptorSetLayout> m_descriptor_set_layout;
	std::shared_ptr<VWrap::Pipeline> m_pipeline;
	std::shared_ptr<VWrap::Sampler> m_sampler;
	std::shared_ptr<VWrap::Buffer> m_vertex_buffer;
	std::shared_ptr<VWrap::Buffer> m_index_buffer;
	std::vector<std::shared_ptr<VWrap::Buffer>> m_uniform_buffers;
	std::vector<void*> m_uniform_buffers_mapped;
	std::shared_ptr<VWrap::DescriptorPool> m_descriptor_pool;
	std::vector<std::shared_ptr<VWrap::DescriptorSet>> m_descriptor_sets;
	std::shared_ptr<VWrap::CommandPool> m_graphics_pool;

	VkExtent2D m_extent;

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

	void LoadModel();

public:

	static std::shared_ptr<MeshRasterizer> Create(std::shared_ptr<VWrap::Device> device, std::shared_ptr<VWrap::RenderPass> render_pass, std::shared_ptr<VWrap::CommandPool> graphics_pool, VkExtent2D extent, uint32_t num_frames);

	/// <summary>
	/// Records the given command buffer to draw to the given framebuffer.
	/// </summary>
	/// <param name="commandBuffer"> The command buffer to record to. </param>
	/// <param name="imageIndex"> The index of the swapchain image to draw to. </param>
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t frame);


	/// <summary>
	/// Updates the uniform buffer for the given frame using perpetually-mapped memory.
	/// </summary>
	/// <param name="frame"> The index of the buffer to update. </param>
	void updateUniformBuffer(uint32_t frame);

	void Resize(VkExtent2D extent) {
		m_extent = extent;
	}
};

