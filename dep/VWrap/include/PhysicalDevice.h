#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include "Utils.h"
#include "Instance.h"
#include "Surface.h"
#include <optional>
#include <set>
#include <string>

namespace VWrap
{

	class PhysicalDevice
	{
	private:
		VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
        std::shared_ptr<Instance> m_instance_ptr;
		std::shared_ptr<Surface> m_surface_ptr;

		bool isPhysicalDeviceSuitable();

	public:
		static std::shared_ptr<PhysicalDevice> Pick(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface);
		QueueFamilyIndices FindQueueFamilies();

        VkPhysicalDevice getHandle() const { return m_physical_device; }

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        SwapchainSupportDetails QuerySwapchainSupport();

		VkSampleCountFlagBits GetMaxUsableSampleCount();

        bool checkDeviceExtensions();

	};
}