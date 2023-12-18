#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include "PhysicalDevice.h"

namespace VWrap {

	class Device
	{
	private:
		
		VkDevice m_device{ VK_NULL_HANDLE };
		std::shared_ptr<PhysicalDevice> m_physical_device_ptr;

	public:

		static std::shared_ptr<Device> Create(std::shared_ptr<PhysicalDevice> physical_device, bool enable_validation_layers);

		VkDevice getHandle() const {
			return m_device;
		}

		std::shared_ptr<PhysicalDevice> getPhysicalDevicePtr() const {
			return m_physical_device_ptr;
		}

		~Device();
			
	};
}