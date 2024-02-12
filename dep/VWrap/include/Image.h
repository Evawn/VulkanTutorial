#pragma once
#include "Vulkan/vulkan.h"
#include "Device.h"
#include <memory>
#include <string>

#include "vk_mem_alloc.h"

#include "Allocator.h"


namespace VWrap {

	/// <summary> Describes parameters for creating an image </summary>
	struct ImageCreateInfo {
		uint32_t width, height, depth;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags properties;
		uint32_t mip_levels;
		VkSampleCountFlagBits samples;
	};

	/// <summary> Represents a Vulkan image and its corresponding memory allocation </summary>
	class Image {

	private:

		/// <summary> The Vulkan image handle </summary>
		VkImage m_image;

		/// <summary> The VMA allocation handle </summary>
		VmaAllocation m_allocation;

		/// <summary> The image format </summary>
		VkFormat m_format;

		/// <summary> The number of mip levels </summary>
		uint32_t m_mip_levels;

		/// <summary> The width and height of the image </summary>
		uint32_t m_width, m_height;

		/// <summary> The allocator used to create the image </summary>
		std::shared_ptr<Allocator> m_allocator;

	public:
		/// <summary> Creates a new image </summary>
		static std::shared_ptr<Image> Create2D(std::shared_ptr<Allocator> allocator,  ImageCreateInfo& info);

		static std::shared_ptr<Image> Create3D(std::shared_ptr<Allocator> allocator, ImageCreateInfo& info);

		/// <summary>
		/// Returns the image handle
		/// </summary>
		VkImage Get() const { return m_image; }

		/// <summary>
		/// Gets the image format
		/// </summary>
		VkFormat GetFormat() const { return m_format; }

		/// <summary>
		/// Gets the number of mip levels
		/// </summary>
		uint32_t GetMipLevels() const { return m_mip_levels; }

		/// <summary>
		/// Gets the allocator used to create the image
		/// </summary>
		std::shared_ptr<Allocator> GetAllocator() const { return m_allocator; }

		~Image();
	};
}
