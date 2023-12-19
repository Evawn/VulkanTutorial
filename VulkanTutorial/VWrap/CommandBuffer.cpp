#include "CommandBuffer.h"

namespace VWrap {

	std::shared_ptr<CommandBuffer> CommandBuffer::Create(std::shared_ptr<CommandPool> command_pool, VkCommandBufferLevel level) {
		auto ret = std::make_shared<CommandBuffer>();
		ret->m_command_pool = command_pool;

		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = command_pool->GetHandle();
		info.level = level;
		info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(command_pool->GetDevice()->GetHandle(), &info, &ret->m_command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		return ret;
	
	}

	std::shared_ptr<CommandBuffer> CommandBuffer::BeginSingleTimeCommands(std::shared_ptr<CommandPool> command_pool) {
		auto ret = Create(command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(ret->m_command_buffer, &info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		return ret;
	}

	void CommandBuffer::EndSingleTimeCommands(std::shared_ptr<CommandBuffer> command_buffer) {
		if (vkEndCommandBuffer(command_buffer->m_command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}

		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &command_buffer->m_command_buffer;

		if (vkQueueSubmit(command_buffer->GetCommandPool()->GetQueue()->GetHandle(), 1, &info, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit command buffer!");
		}

		vkQueueWaitIdle(command_buffer->GetCommandPool()->GetQueue()->GetHandle());
	}
}