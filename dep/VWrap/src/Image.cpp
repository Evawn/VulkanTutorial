#include "Image.h"

namespace VWrap {

	std::shared_ptr<Image> Image::Create2D(std::shared_ptr<Allocator> allocator, ImageCreateInfo& info) {
		auto ret = std::make_shared<Image>();
		ret->m_allocator = allocator;
		ret->m_format = info.format;
		ret->m_mip_levels = info.mip_levels == 0 ? 1 : info.mip_levels;
		ret->m_width = info.width;
		ret->m_height = info.height;

		VkSampleCountFlagBits samples = info.samples ? info.samples : VK_SAMPLE_COUNT_1_BIT;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = info.width;
		imageInfo.extent.height = info.height;
		imageInfo.extent.depth = 1;
		imageInfo.format = info.format;
		imageInfo.arrayLayers = 1;
		imageInfo.mipLevels = ret->m_mip_levels;
		imageInfo.tiling = info.tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = info.usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = samples;
		imageInfo.flags = 0; // Optional

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		if (vmaCreateImage(allocator->Get(), &imageInfo, &allocCreateInfo, &ret->m_image, &ret->m_allocation, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		return ret;
	}

	std::shared_ptr<Image> Image::Create3D(std::shared_ptr<Allocator> allocator, ImageCreateInfo& info)
	{
		auto ret = std::make_shared<Image>();
		ret->m_allocator = allocator;
		ret->m_format = info.format;
		ret->m_mip_levels = info.mip_levels == 0 ? 1 : info.mip_levels;
		ret->m_width = info.width;
		ret->m_height = info.height;

		VkSampleCountFlagBits samples = info.samples ? info.samples : VK_SAMPLE_COUNT_1_BIT;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
		imageInfo.extent.width = info.width;
		imageInfo.extent.height = info.height;
		imageInfo.extent.depth = info.depth;
		imageInfo.format = info.format;
		imageInfo.arrayLayers = 1;
		imageInfo.mipLevels = ret->m_mip_levels;
		imageInfo.tiling = info.tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = info.usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = samples;
		imageInfo.flags = 0; // Optional

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		if (vmaCreateImage(allocator->Get(), &imageInfo, &allocCreateInfo, &ret->m_image, &ret->m_allocation, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		return ret;
	}

	//std::shared_ptr<Image> Image::Texture2DFromFile(std::shared_ptr<Allocator> allocator,std::shared_ptr<CommandPool> graphics_pool, const char* file_name) {
	//	int texWidth, texHeight, texChannels;
	//	stbi_uc* pixels = stbi_load(file_name, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	//	VkDeviceSize imageSize = texWidth * texHeight * 4;

	//	if (!pixels) {
	//		throw std::runtime_error("Failed to load images!");
	//	}

	//	auto staging_buffer = Buffer::CreateStaging(allocator, imageSize);

	//	void* data;
	//	vmaMapMemory(allocator->Get(), staging_buffer->GetAllocation(), &data);
	//	memcpy(data, pixels, static_cast<size_t>(imageSize));
	//	vmaUnmapMemory(allocator->Get(), staging_buffer->GetAllocation());
	//	stbi_image_free(pixels);

	//	VWrap::ImageCreateInfo info{};
	//	info.width = static_cast<uint32_t>(texWidth);
	//	info.height = static_cast<uint32_t>(texHeight);
	//	info.format = VK_FORMAT_R8G8B8A8_SRGB;
	//	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	//	info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	//	info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	//	info.mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	//	auto ret = Image::Create(allocator, graphics_pool, info);

	//	CommandBuffer::TransitionLayout(graphics_pool, ret, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	//	CommandBuffer::CopyBufferToImage(graphics_pool, staging_buffer, ret, texWidth, texHeight);
	//	CommandBuffer::GenerateMipmaps(graphics_pool, ret, texWidth, texHeight);

	//	return ret;
	//}





	Image::~Image() {
		if (m_image != VK_NULL_HANDLE && m_allocation != nullptr)
			vmaDestroyImage(m_allocator->Get(), m_image, m_allocation);
	}
}