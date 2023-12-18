#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "Utils.h"
#include <array>

namespace VWrap {

	class Pipeline {
	private:
		VkPipeline m_pipeline;
		VkPipelineLayout m_pipeline_layout;

		std::shared_ptr<Device> m_device_ptr;

		static VkShaderModule CreateShaderModule(std::shared_ptr<Device> device, const std::vector<char>& code);

	public:

		static std::shared_ptr<Pipeline> Create(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<DescriptorSetLayout> descriptor_set_layout, VkExtent2D extent);;

		VkPipeline GetHandle() const { return m_pipeline; }

		VkPipelineLayout GetLayoutHandle() const { return m_pipeline_layout; }

		~Pipeline();

	};
}