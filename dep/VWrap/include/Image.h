#pragma once
#include "Vulkan/vulkan.h"
#include "Device.h"
#include <memory>
#include <string>
#include "stb_image.h"
#include "vk_mem_alloc.h"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Allocator.h"


namespace VWrap {

	/// <summary> Describes parameters for creating an image </summary>
	struct ImageCreateInfo {
		uint32_t width, height;
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

		/// <summary> The command pool used to operate on the image </summary>
		std::shared_ptr<CommandPool> m_command_pool;

		/// <summary> Finds a suitable format among a list of candidates for the given device, tiling, and format features </summary>
		static VkFormat FindSupportedFormat(std::shared_ptr<Device> device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		/// <summary> Checks if the given format has a stencil component </summary>
		static bool HasStencilComponent(VkFormat format);

	public:
		/// <summary> Creates a new image </summary>
		static std::shared_ptr<Image> Create(std::shared_ptr<Allocator> allocator, std::shared_ptr<CommandPool> graphics_pool, ImageCreateInfo& info);

		/// <summary> Creates a new image from a file, configured as a 2D texture </summary>
		static std::shared_ptr<Image> Texture2DFromFile(std::shared_ptr<Allocator> allocator, std::shared_ptr<CommandPool> graphics_pool, const char* file_name);

		/// <summary> Finds a suitable depth format for the given device </summary>
		static VkFormat FindDepthFormat(std::shared_ptr<Device> device);

		/// <summary>
		/// Submits a pipeline barrier to transition the image layout
		/// </summary>
		void TransitionLayout(VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

		/// <summary>
		/// Copies the contents of a buffer to the image
		/// </summary>
		void CopyFromBuffer(VkBuffer src, uint32_t width, uint32_t height);

		/// <summary>
		/// Generates mipmaps for the image
		/// </summary>
		void GenerateMipmaps();

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

		~Image();
	};
}
