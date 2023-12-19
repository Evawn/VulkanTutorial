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
		std::shared_ptr<RenderPass> m_render_pass_ptr;
		std::vector<std::shared_ptr<ImageView>> m_attachments;
		VkExtent2D m_extent;

	public:

		static std::shared_ptr<Framebuffer> Create2D(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<ImageView> color_attachment, std::shared_ptr<ImageView> depth_attachment, VkExtent2D extent);

		VkFramebuffer GetHandle() const { return m_framebuffer; }

		std::shared_ptr<RenderPass> GetRenderPass() const { return m_render_pass_ptr; }
		std::vector<std::shared_ptr<ImageView>> GetAttachments() const { return m_attachments; }
		VkExtent2D GetExtent() const { return m_extent; }

		~Framebuffer();

	};
}