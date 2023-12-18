#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"

namespace VWrap {
	class Queue
	{
	private:

		VkQueue m_queue;
		uint32_t m_queue_family_index;

		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<Queue> Create(std::shared_ptr<Device> device, uint32_t queue_family_index);

		VkQueue GetHandle() const { return m_queue; }

		std::shared_ptr<Device> GetDevicePtr() const { return m_device_ptr; }

		uint32_t GetQueueFamilyIndex() const { return m_queue_family_index; }

	};

}

