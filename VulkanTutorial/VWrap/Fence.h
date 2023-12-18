#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"

namespace VWrap {
	class Fence
	{
	private:

		VkFence m_fence;
		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<Fence> Create(std::shared_ptr<Device> device);

		VkFence GetHandle() const { return m_fence; }

		~Fence();
	};

}
