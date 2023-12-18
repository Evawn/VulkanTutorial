#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

namespace VWrap {
	class Buffer
	{
	private:

		VkBuffer m_buffer;
		VkDeviceMemory m_buffer_memory;

		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<Buffer> Create(std::shared_ptr<Device> device,
												VkDeviceSize size,
												VkBufferUsageFlags usage,
												VkMemoryPropertyFlags properties);

		static std::shared_ptr<Buffer> CreateStaging(std::shared_ptr<Device> device,
			VkDeviceSize size);

		void CopyFromBuffer(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, VkDeviceSize size);

		VkBuffer GetHandle() const { return m_buffer; }

		VkDeviceMemory GetMemory() const { return m_buffer_memory; }

		~Buffer();
	};

}
