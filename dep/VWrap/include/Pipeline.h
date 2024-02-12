#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "Utils.h"
#include <array>

namespace VWrap {

	/// <summary>
	/// Represents a Vulkan pipeline.
	/// </summary>
	class Pipeline {
	private:

		/// <summary> The pipeline handle. </summary>
		VkPipeline m_pipeline;

		/// <summary> The pipeline layout handle. </summary>
		VkPipelineLayout m_pipeline_layout;

		/// <summary> The device that created the pipeline. </summary>
		std::shared_ptr<Device> m_device;

		/// <summary>
		/// Creates a shader module from the given code.
		/// </summary>
		static VkShaderModule CreateShaderModule(std::shared_ptr<Device> device, const std::vector<char>& code);

	public:

		/// <summary>
		/// Creates a new pipeline with the given render pass, descriptor set layout and extent.
		/// </summary>
		static std::shared_ptr<Pipeline> CreateRasterizer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<DescriptorSetLayout> descriptor_set_layout, VkExtent2D extent);;

		/// <summary>
		/// Creates a new pipeline with the given render pass, descriptor set layout and extent.
		/// </summary>
		static std::shared_ptr<Pipeline> CreateTracer(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<DescriptorSetLayout> descriptor_set_layout, VkExtent2D extent);;

		/// <summary> Gets the pipeline handle. </summary>
		VkPipeline Get() const { return m_pipeline; }

		/// <summary> Gets the pipeline layout handle. </summary>
		VkPipelineLayout GetLayout() const { return m_pipeline_layout; }

		~Pipeline();
	};
}