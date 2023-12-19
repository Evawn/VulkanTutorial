#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

namespace VWrap {

	/// <summary>
	/// Represents a VkBuffer and its associated VkDeviceMemory.
	/// </summary>
	class Buffer
	{
	private:

		/// <summary>
		/// The underlying buffer handle.
		/// </summary>
		VkBuffer m_buffer;

		/// <summary>
		/// The underlying buffer memory handle.
		/// </summary>
		VkDeviceMemory m_buffer_memory;

		/// <summary>
		/// The device that created this buffer.
		/// </summary>
		std::shared_ptr<Device> m_device;

	public:

		/// <summary>
		/// Creates a buffer with the given parameters.
		/// </summary>
		/// <param name="device"> The device to create the buffer </param>
		/// <param name="size"> The size of the buffer </param>
		/// <param name="usage"> Buffer usage flags </param>
		/// <param name="properties"> Memory property flags </param>
		/// <returns> A pointer to the created Buffer </returns>
		static std::shared_ptr<Buffer> Create(std::shared_ptr<Device> device,
												VkDeviceSize size,
												VkBufferUsageFlags usage,
												VkMemoryPropertyFlags properties);

		/// <summary>
		/// Creates a buffer with parameters suited for staging
		/// </summary>
		/// <param name="device"> The device to create the buffer  </param>
		/// <param name="size"> The size of the buffer </param>
		/// <returns> A pointer to the staging buffer </returns>
		static std::shared_ptr<Buffer> CreateStaging(std::shared_ptr<Device> device,
			VkDeviceSize size);

		/// <summary>
		/// Copies the data from the source buffer to this buffer
		/// </summary>
		/// <param name="command_pool"> The command pool representing the queue to submit the transfer on </param>
		/// <param name="src_buffer"> The buffer whose data will be copied </param>
		/// <param name="size"> The size of the copy area </param>
		void CopyFromBuffer(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, VkDeviceSize size);


		/// <summary>
		/// Gets the underlying buffer handle.
		/// </summary>
		VkBuffer GetHandle() const { return m_buffer; }

		/// <summary>
		/// Gets the underlying buffer memory handle.
		/// </summary>
		VkDeviceMemory GetMemory() const { return m_buffer_memory; }

		/// <summary>
		/// Destroys the underlying VkBuffer and frees the associated VkDeviceMemory.
		/// </summary>
		~Buffer();
	};

}
