#include "Framebuffer.h"

namespace VWrap {
	std::shared_ptr<Framebuffer> Framebuffer::Create2D(std::shared_ptr<Device> device, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<ImageView> color_attachment, std::shared_ptr<ImageView> depth_attachment, VkExtent2D extent) {
		auto ret = std::make_shared<Framebuffer>();
		ret->m_device = device;
		ret->m_extent = extent;
		ret->m_render_pass = render_pass;
		ret->m_attachments.push_back(color_attachment);
		ret->m_attachments.push_back(depth_attachment);

		VkFramebufferCreateInfo createInfo{};
		
		createInfo.renderPass = render_pass->GetHandle();
		createInfo.height = extent.height;
		createInfo.width = extent.width;
		createInfo.layers = 1;
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		std::array<VkImageView, 2> image_view_handles = { color_attachment->GetHandle(), depth_attachment->GetHandle()};
		createInfo.attachmentCount = static_cast<uint32_t>(image_view_handles.size());
		createInfo.pAttachments = image_view_handles.data();

		if (vkCreateFramebuffer(device->GetHandle(), &createInfo, nullptr, &ret->m_framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}

		return ret;
	}

	Framebuffer::~Framebuffer() {
		if(m_framebuffer != VK_NULL_HANDLE) 
			vkDestroyFramebuffer(m_device->GetHandle(), m_framebuffer, nullptr);
	}
}
