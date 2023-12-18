#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "RenderPass.h"
#include "ImageView.h"
#include <array>

namespace VWrap {

	class Framebuffer {
	private:
		VkFramebuffer m_framebuffer;
		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<Framebuffer> Create2D(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<ImageView> color_attachment, std::shared_ptr<ImageView> depth_attachment, VkExtent2D extent);

		VkFramebuffer GetHandle() const { return m_framebuffer; }

		~Framebuffer();

	};
}