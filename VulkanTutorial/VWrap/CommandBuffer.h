#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "CommandPool.h"

namespace VWrap {
	class CommandBuffer
	{
	private:

		VkCommandBuffer m_command_buffer{ VK_NULL_HANDLE };
		std::shared_ptr<CommandPool> m_command_pool_ptr;

	public:

		static std::shared_ptr<CommandBuffer> Create(std::shared_ptr<CommandPool> command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		static std::shared_ptr<CommandBuffer> BeginSingleTimeCommands(std::shared_ptr<CommandPool> command_pool);

		static void EndSingleTimeCommands(std::shared_ptr<CommandBuffer> command_buffer);

		VkCommandBuffer GetHandle() const { return m_command_buffer; }

		std::shared_ptr<CommandPool> GetCommandPool() const { return m_command_pool_ptr; }

		~CommandBuffer();
	};

}