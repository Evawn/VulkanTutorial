#pragma once
#include "Vulkan/vulkan.h"
#include "Device.h"
#include "Image.h"
#include <memory>

namespace VWrap {

	class RenderPass {

	private:

		VkRenderPass m_render_pass;
		
		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<RenderPass> Create(std::shared_ptr<Device> device, VkFormat format);

		VkRenderPass GetHandle() const { return m_render_pass; }

		~RenderPass();
	};
}
