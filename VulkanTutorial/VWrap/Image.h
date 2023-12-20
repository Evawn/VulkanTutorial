#pragma once
#include "Vulkan/vulkan.h"
#include "Device.h"
#include <memory>
#include <string>
#include "stb_image.h"
#include "CommandBuffer.h"
#include "Buffer.h"

namespace VWrap {

	struct ImageCreateInfo {
		uint32_t width, height;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags properties;
		uint32_t mip_levels;
		VkSampleCountFlagBits samples;
	};

	class Image {

	private:

		VkImage m_image;
		VkDeviceMemory m_image_memory;
		VkFormat m_format;
		uint32_t m_mip_levels;
		uint32_t m_width, m_height;

		std::shared_ptr<Device> m_device_ptr;
		std::shared_ptr<CommandPool> m_graphics_pool_ptr;

		static VkFormat FindSupportedFormat(std::shared_ptr<Device> device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		static bool HasStencilComponent(VkFormat format);

	public:

		static std::shared_ptr<Image> Create(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> graphics_pool, ImageCreateInfo& info);

		static std::shared_ptr<Image> Texture2DFromFile(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> graphics_pool, const char* file_name);

		//static std::shared_ptr<Image> CreateDepthImage(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> graphics_pool, VkExtent2D extent);

		static VkFormat FindDepthFormat(std::shared_ptr<Device> device);

		void TransitionLayout(VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

		void CopyFromBuffer(VkBuffer src, uint32_t width, uint32_t height);

		void GenerateMipmaps();

		VkImage GetHandle() const { return m_image; }

		VkDeviceMemory GetMemoryHandle() const { return m_image_memory; }

		VkFormat GetFormat() const { return m_format; }

		uint32_t GetMipLevels() const { return m_mip_levels; }

		~Image();
	};
}
