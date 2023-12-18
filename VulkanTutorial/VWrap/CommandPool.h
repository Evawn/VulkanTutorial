#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "Queue.h"

namespace VWrap{
	class CommandPool
	{
	private:

		VkCommandPool m_command_pool;

		std::shared_ptr<Device> m_device_ptr; 
		std::shared_ptr<Queue> m_queue_ptr;

	public:

		static std::shared_ptr<CommandPool> Create(std::shared_ptr<Device> device, std::shared_ptr<Queue>);

		VkCommandPool GetHandle() const { return m_command_pool; }

		std::shared_ptr<Device> GetDevicePtr() const { return m_device_ptr; }

		std::shared_ptr<Queue> GetQueuePtr() const { return m_queue_ptr; }

		~CommandPool();
	};
	
}

