#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "CommandPool.h"

namespace VWrap {

	/// <summary>
	/// Represents a command buffer. Contains static methods for common command buffer operations.
	/// </summary>
	class CommandBuffer
	{
	private:

		/// <summary>
		/// The underlying vulkan command buffer handle.
		/// </summary>
		VkCommandBuffer m_command_buffer{ VK_NULL_HANDLE };

		/// <summary>
		/// The command pool that this command buffer was allocated from.
		/// </summary>
		std::shared_ptr<CommandPool> m_command_pool;

	public:

		/// <summary>
		/// Allocates a new command buffer from the given pool.
		/// </summary>
		/// <param name="command_pool">The command pool to allocate the command buffer from.</param>
		/// <param name="level">The level of the command buffer.</param>
		/// <returns>A shared pointer to the command buffer.</returns>
		static std::shared_ptr<CommandBuffer> Create(std::shared_ptr<CommandPool> command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		/// <summary>
		/// Allocates a single-use command buffer from the given command pool, and begins recording.
		/// </summary>
		/// <param name="command_pool">The command pool to allocate the command buffer from.</param>
		/// <returns>A shared pointer to the command buffer.</returns>
		static std::shared_ptr<CommandBuffer> BeginSingleTimeCommands(std::shared_ptr<CommandPool> command_pool);

		/// <summary>
		/// Ends recording of the given command buffer, and submits it to the given queue.
		/// </summary>
		static void EndSingleTimeCommands(std::shared_ptr<CommandBuffer> command_buffer);

		/// <summary>
				/// Gets the underlying vulkan command buffer handle.
				/// </summary>
		VkCommandBuffer GetHandle() const { return m_command_buffer; }

		/// <summary>
		/// Gets the command pool that this command buffer was allocated from.
		/// </summary>
		std::shared_ptr<CommandPool> GetCommandPool() const { return m_command_pool; }
	};
}