#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "Image.h"

namespace VWrap {

	class ImageView {
	private:
		VkImageView m_image_view{ VK_NULL_HANDLE };
		std::shared_ptr<Device> m_device_ptr;
		std::shared_ptr<Image> m_image_ptr;

	public:

		static std::shared_ptr<ImageView> Create(std::shared_ptr<Device> device, std::shared_ptr<Image> image, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT);

		static std::shared_ptr<ImageView> Create(std::shared_ptr<Device> device, VkImage image, VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip_levels = 1);

		~ImageView();

		VkImageView GetHandle() const { return m_image_view; }
	};
}