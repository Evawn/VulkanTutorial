#include "ImageView.h"

namespace VWrap {

    std::shared_ptr<ImageView> ImageView::Create(std::shared_ptr<Device> device, std::shared_ptr<Image> image, VkImageAspectFlags aspect) {
        auto ret = Create(device, image->Get(), image->GetFormat(), aspect, image->GetMipLevels());
        ret->m_image = image;
        return ret; 
    }

    std::shared_ptr<ImageView> ImageView::Create(std::shared_ptr<Device> device, VkImage image, VkFormat format, VkImageAspectFlags aspect, uint32_t mip_levels) {
        auto ret = std::make_shared<ImageView>();
        ret->m_device = device;

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.format = format;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.subresourceRange.aspectMask = aspect;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.levelCount = mip_levels;

        if (vkCreateImageView(device->GetHandle(), &createInfo, nullptr, &ret->m_image_view) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view!");
        }
        return ret;
    }

    ImageView::~ImageView() {
        if (m_image_view != VK_NULL_HANDLE)
		    vkDestroyImageView(m_device->GetHandle(), m_image_view, nullptr);
	}
}