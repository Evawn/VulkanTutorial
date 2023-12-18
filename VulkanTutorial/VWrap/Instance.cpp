#include "Instance.h"


/// <summary>
/// Gets the required extensions and layers, and creates a VkInstance and store it in the 'instance' class member.
/// </summary>
std::shared_ptr<VWrap::Instance> VWrap::Instance::Create(bool enableValidationLayers) {
    auto ret = std::make_shared<Instance>();

    bool validationLayerSupported = checkValidationLayerSupport();

    if (enableValidationLayers && !validationLayerSupported) {
        throw std::runtime_error("Missing support for requested validation layers!");
    }

    // Define ApplicationInfo struct
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Define InstanceCreateInfo struct
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get required Extensions
    auto extensions = getRequiredExtensions(enableValidationLayers);

    // Specify Extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{ };
    // Specify Validation Layers
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // Finally, create the vkInstance
    if (vkCreateInstance(&createInfo, nullptr, &ret->m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }

    // Enable Debug Messenger
    if (enableValidationLayers) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(ret->m_instance, &createInfo, nullptr, &ret->m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger.");
        }
    }

    return ret;
}

VWrap::Instance::~Instance() {
    if (m_instance != VK_NULL_HANDLE) {
        if (m_debugMessenger != VK_NULL_HANDLE) {
            DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }
		vkDestroyInstance(m_instance, nullptr);
	}
}

