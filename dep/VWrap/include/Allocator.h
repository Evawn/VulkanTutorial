#pragma once
#include "vk_mem_alloc.h"
#include <memory>

namespace VWrap
{
	class Allocator
	{
	private:
		VmaAllocator m_allocator;

	public:
		static std::shared_ptr<Allocator> Create(std::shared_ptr<Instance> instance, std::shared_ptr<PhysicalDevice> physical_device, std::shared_ptr<Device> device) {
			auto ret = std::make_shared<Allocator>();

			VmaAllocatorCreateInfo allocatorCreateInfo = {};
			allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
			allocatorCreateInfo.physicalDevice = physical_device->getHandle();
			allocatorCreateInfo.device = device->GetHandle();
			allocatorCreateInfo.instance = instance->getHandle();
			allocatorCreateInfo.pVulkanFunctions = nullptr;

			vmaCreateAllocator(&allocatorCreateInfo, &ret->m_allocator);
			return ret;
		}

		VmaAllocator Get() {
			return m_allocator;
		}

		~Allocator() {
			vmaDestroyAllocator(m_allocator);
		}
	};
}
