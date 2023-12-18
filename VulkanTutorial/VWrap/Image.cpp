#include "Image.h"


namespace VWrap {

    std::shared_ptr<Image> Image::Create(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> graphics_pool, ImageCreateInfo& info) {
		auto ret = std::make_shared<Image>();
        ret->m_device_ptr = device;
        ret->m_graphics_pool_ptr = graphics_pool;
        ret->m_format = info.format;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = info.width;
        imageInfo.extent.height = info.height;
        imageInfo.extent.depth = 1;
        imageInfo.format = info.format;
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = 1;
        imageInfo.tiling = info.tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = info.usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        if (vkCreateImage(device->getHandle(), &imageInfo, nullptr, &ret->m_image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        VkMemoryRequirements imageMemReq;
        vkGetImageMemoryRequirements(device->getHandle(), ret->m_image, &imageMemReq);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = imageMemReq.size;
        allocInfo.memoryTypeIndex = device->getPhysicalDevicePtr()->FindMemoryType(imageMemReq.memoryTypeBits, info.properties);

        if (vkAllocateMemory(device->getHandle(), &allocInfo, nullptr, &ret->m_image_memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate device memory for image!");
        }
        vkBindImageMemory(device->getHandle(), ret->m_image, ret->m_image_memory, 0);

		return ret;
	}

    std::shared_ptr<Image> Image::Texture2DFromFile(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> graphics_pool, const char* file_name) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(file_name, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("Failed to load images!");
        }

        auto staging_buffer = Buffer::CreateStaging(device, imageSize);

        void* data;
        vkMapMemory(device->getHandle(), staging_buffer->GetMemory(), 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device->getHandle(), staging_buffer->GetMemory());
        stbi_image_free(pixels);

        VWrap::ImageCreateInfo info{};
        info.width = static_cast<uint32_t>(texWidth);
        info.height = static_cast<uint32_t>(texHeight);
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        auto ret = Image::Create(device, graphics_pool, info);

        ret->TransitionLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        ret->CopyFromBuffer(staging_buffer->GetHandle(), texWidth, texHeight);
        ret->TransitionLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return ret;
    }

    std::shared_ptr<Image> Image::CreateDepthImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> graphics_pool, VkExtent2D extent)
    {
        VkFormat depthFormat = FindDepthFormat(device);

        ImageCreateInfo info{};
        info.format = depthFormat;
        info.width = extent.width;
        info.height = extent.height;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        return Create(device, graphics_pool, info);
    }

    VkFormat Image::FindSupportedFormat(std::shared_ptr<Device> device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevicePtr()->getHandle(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
        }

    VkFormat Image::FindDepthFormat(std::shared_ptr<Device> device)
    {
        return Image::FindSupportedFormat(
            device,
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool Image::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void Image::TransitionLayout(VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.newLayout = new_layout;
        barrier.oldLayout = old_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (Image::HasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        // use graphics command buffer here

        auto graphics_command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(m_graphics_pool_ptr);
        vkCmdPipelineBarrier(
            graphics_command_buffer->GetHandle(),
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        VWrap::CommandBuffer::EndSingleTimeCommands(graphics_command_buffer);
    }

    void Image::CopyFromBuffer(VkBuffer src, uint32_t width, uint32_t height) {
        auto command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(m_graphics_pool_ptr);

        VkBufferImageCopy copy{};
        copy.bufferOffset = 0;
        copy.imageOffset = { 0, 0,0 };
        copy.bufferRowLength = 0;
        copy.bufferImageHeight = 0;

        copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.mipLevel = 0;
        copy.imageSubresource.baseArrayLayer = 0;
        copy.imageSubresource.layerCount = 1;
        copy.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(command_buffer->GetHandle(), src, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        VWrap::CommandBuffer::EndSingleTimeCommands(command_buffer);
    }


    Image::~Image() {
        if (m_image != VK_NULL_HANDLE) 
			vkDestroyImage(m_device_ptr->getHandle(), m_image, nullptr);
			
		
		if (m_image_memory != VK_NULL_HANDLE) 
            vkFreeMemory(m_device_ptr->getHandle(), m_image_memory, nullptr);
    }
}