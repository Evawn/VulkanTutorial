#include "Device.h"

namespace VWrap {

	std::shared_ptr<Device> Device::Create(std::shared_ptr<PhysicalDevice> physical_device,
											bool enable_validation_layers) {

		auto ret = std::make_shared<Device>();
		ret->m_physical_device_ptr = physical_device;

        QueueFamilyIndices indices = physical_device->FindQueueFamilies();

        // Create info for each unique queue family
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies =
        { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() }; // only holds unique values

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        // List of required device features
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        // Create info for logical device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Specify device-specific extensions
        createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
        createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

        // Enable layers
        if (enable_validation_layers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // Create the logical device
        if (vkCreateDevice(physical_device->getHandle(), &createInfo, nullptr, &ret->m_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        // Get handle for queue
        /*vkGetDeviceQueue(ret->m_device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(ret->m_device, indices.presentFamily.value(), 0, &presentQueue);
        vkGetDeviceQueue(ret->m_device, indices.transferFamily.value(), 0, &transferQueue);*/
		return ret;
	
	}

	Device::~Device() {
		if (m_device != VK_NULL_HANDLE) {
			vkDestroyDevice(m_device, nullptr);
		}
	}
}