#include "PhysicalDevice.h"


namespace VWrap
{
	std::shared_ptr<PhysicalDevice> PhysicalDevice::Pick(std::shared_ptr<Instance> instance, std::shared_ptr<Surface> surface)
    {
        auto ret = std::make_shared<PhysicalDevice>();
        ret->m_instance_ptr = instance;
        ret->m_surface_ptr = surface;

        // Query physical device count
        uint32_t pdCount = 0;
        vkEnumeratePhysicalDevices(instance->getHandle(), &pdCount, nullptr);
        if (pdCount == 0) {
            throw std::runtime_error("No physical devices!");
        }

        // Query physical devices
        std::vector<VkPhysicalDevice> devices(pdCount);
        vkEnumeratePhysicalDevices(instance->getHandle(), &pdCount, devices.data());

        for (const auto& device : devices)
        {
            ret->m_physical_device = device;
            if (ret->isPhysicalDeviceSuitable()) break;
            else ret->m_physical_device = VK_NULL_HANDLE;
        }

        if (ret->m_physical_device == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find suitable physical device!");
        }
        return ret;
    }

    /// <summary>
/// Queries required device features and decides if suitable.
/// </summary>
/// <param name="device"></param>
/// <returns> Whether the device is suitable.</returns>
    bool PhysicalDevice::isPhysicalDeviceSuitable()
    {
        QueueFamilyIndices indices = FindQueueFamilies();
        bool extensionsSupported = checkDeviceExtensions();

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(m_physical_device, &supportedFeatures);

        bool swapchainSupported = false;
        if (extensionsSupported) {
            SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport();
            swapchainSupported = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapchainSupported && supportedFeatures.samplerAnisotropy;
    }

    /// <summary>
/// Queries 'device' for support for extensions defines in DEVICE_EXTENSIONS.
/// </summary>
/// <param name="device"> The physical device to query the extension support of.</param>
/// <returns>Whether or not 'device' supports all extensions found in DEVICE_EXTENSIONS</returns>
    bool PhysicalDevice::checkDeviceExtensions() {

        // First get the number of available extensions, create a vec, then get the extensions
        uint32_t availExtensionCount;
        vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &availExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availExtensions(availExtensionCount);
        vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &availExtensionCount, availExtensions.data());

        // Get a list of all unique required extensions
        std::set<std::string> uniqueRequiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
        // If an available extension is required, remove it from the required list
        for (VkExtensionProperties prop : availExtensions) {
            uniqueRequiredExtensions.erase(prop.extensionName);
        }
        return uniqueRequiredExtensions.empty();
    }

    /// <summary>
    /// Queries 'device' for supported swapchain functionality and returns its support.
    /// </summary>
    /// <param name="device"> The device whose swapchain support is to be queried. </param>
    /// <returns> A struct containing details about the swapchain support of 'device'.</returns>
    SwapchainSupportDetails PhysicalDevice::QuerySwapchainSupport() {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface_ptr->getHandle(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface_ptr->getHandle(), &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface_ptr->getHandle(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface_ptr->getHandle(), &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface_ptr->getHandle(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    /// <summary>
/// Queries physical device for the indices of required queue families.
/// </summary>
/// <param name="device">The physical device to be queried</param>
/// <returns>A struct containing indices for all required queue families.</returns>
    QueueFamilyIndices PhysicalDevice::FindQueueFamilies() {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(this->m_physical_device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(this->m_physical_device, &queueFamilyCount, queueFamilyProperties.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilyProperties) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                indices.transferFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(this->m_physical_device, i, this->m_surface_ptr->getHandle(), &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

    uint32_t PhysicalDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (typeFilter & (1 << i) && \
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties
                ) {
                return i;
            }
        }
        throw std::runtime_error("Could not find suitable memory type!");
    }
}

