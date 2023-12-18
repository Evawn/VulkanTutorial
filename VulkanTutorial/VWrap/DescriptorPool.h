#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"

namespace VWrap {
	class DescriptorPool
	{
	private:

		VkDescriptorPool m_descriptor_pool;
		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<DescriptorPool> Create(std::shared_ptr<Device> device, uint32_t num_frames);

		VkDescriptorPool GetHandle() const { return m_descriptor_pool; }

		std::shared_ptr<Device> GetDevicePtr() const { return m_device_ptr; }

		~DescriptorPool();
	};

}
