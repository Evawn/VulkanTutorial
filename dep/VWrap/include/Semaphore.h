#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"

namespace VWrap {
	class Semaphore
	{
	private:

		VkSemaphore m_semaphore;
		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<Semaphore> Create(std::shared_ptr<Device> device);

		VkSemaphore GetHandle() const { return m_semaphore; }

		~Semaphore();
	};

}
