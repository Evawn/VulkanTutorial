#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "vk_mem_alloc.h"
#include "Allocator.h"

namespace VWrap {

	/// <summary>
	/// Represents a VkBuffer and its associated VmaAllocation.
	/// </summary>
	class Buffer
	{
	private:

		/// <summary>
		/// The underlying buffer handle.
		/// </summary>
		VkBuffer m_buffer;

		VmaAllocation m_allocation;

		/// <summary>
		/// The device that created this buffer.
		/// </summary>
		std::shared_ptr<Device> m_device;

		std::shared_ptr<Allocator> m_allocator;

	public:

		static std::shared_ptr<Buffer> Create(std::shared_ptr<Allocator> allocator, std::shared_ptr<Device> device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VmaAllocationCreateFlags flags);

		/// <summary>
		/// Creates a buffer with the given parameters.
		/// </summary>
		static std::shared_ptr<Buffer> Create(std::shared_ptr<Allocator> allocator, std::shared_ptr<Device> device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties);

		/// <summary>
		/// Creates a buffer with parameters suited for staging
		/// </summary>
		static std::shared_ptr<Buffer> CreateStaging(std::shared_ptr<Allocator> allocator,
			std::shared_ptr<Device> device,
			VkDeviceSize size);

		/// <summary>
		/// Creates a buffer with parameters suited for persistent mapped memory
		/// </summary>
		static std::shared_ptr<Buffer> CreateMapped(std::shared_ptr<Allocator> allocator, std::shared_ptr<Device> device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			void*& data);

		/// <summary>
		/// Copies the data from the source buffer to this buffer
		/// </summary>
		void CopyFromBuffer(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, VkDeviceSize size);

		/// <summary>
		/// Gets the underlying buffer handle.
		/// </summary>
		VkBuffer GetHandle() const { return m_buffer; }

		VmaAllocation GetAllocation() const { return m_allocation; }

		/// <summary>
		/// Destroys the underlying VkBuffer and VmaAllocation.
		/// </summary>
		~Buffer();
	};

}
