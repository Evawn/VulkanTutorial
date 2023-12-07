#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <optional>
#include <set>
#include <algorithm>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

/// <summary>
/// Proxy function that loads the extension function vkCreateDebugUtilsMessengerEXT and then runs it.
/// </summary>
/// <param name="instance"> The instance to look up the address of the func. </param>
/// <param name="pCreateInfo"> Our create struct. </param>
/// <param name="pAllocator"> Custom Allocator. </param>
/// <param name="pDebugMessenger">  Where to store the callback handle. </param>
/// <returns> Whether the function was successfully loaded and ran. </returns>
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger) {

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) 
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/// <summary>
/// Proxy function that loads the EXT function 'vkDestroyDebugUtilsMessengerEXT' and then runs it.
/// </summary>
/// <param name="instance"> The instance containing the custom callback. </param>
/// <param name="debugMessenger"> The Vulkan handle for the custom callback. </param>
/// <param name="pAllocator"> Custom allocator. </param>
void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class HelloTriangleApplication {
// CLASS TYPES -------------------------------------------------------------
private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

// CLASS MEMBERS -----------------------------------------------------------
public:
private:
    GLFWwindow* window;

// VULKAN OBJECTS
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Implicitly destroyed
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;

// CLASS FUNCTIONS ---------------------------------------------------------
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Tutorial", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        for (VkImageView view : swapchainImageViews) {
            vkDestroyImageView(device, view, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyDevice(device, nullptr);
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(
                instance,
                debugMessenger,
                nullptr
            );
        }
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    /// <summary>
    /// Gets the required extensions and layers, and creates a VkInstance and store it in the 'instance' class member.
    /// </summary>
    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
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
        auto extensions = getRequiredExtensions();

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
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance!");
        }
    }

    /// <summary>
    /// Queries Vulkan for available validation layers, and compares it with the elements of VALDIATION_LAYERS.
    /// </summary>
    /// <returns> Whether all elements of VALIDATION_LAYERS are available. </returns>
    bool checkValidationLayerSupport() {
        uint32_t layerCount = 0;

        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const auto& layerName : VALIDATION_LAYERS) {
            bool found = false;
            for (const auto& layer : availableLayers) {
                if (strcmp(layer.layerName, layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }

        return true;
    }

    /// <summary>
    /// Queries GLFW for required extensions, and queries Vulkan for available extentions.
    /// If the GLFW extensions are not available, it will print it out.
    /// </summary>
    /// <returns> The required extensions, including validation extensions if enabled. </returns>
    std::vector<const char*> getRequiredExtensions() {

        // Get the names of the required instance extensions from GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // Store the names as a vector<string> for later comparison
        std::vector<std::string> glfwExtStrings;
        for (int i = 0; i < glfwExtensionCount; i++) {
            glfwExtStrings.push_back(glfwExtensions[i]);
        }

        // Get and store the available instance extensions
        uint32_t availableExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

        // Output available extensions and store in set
        std::cout << "Available Extensions:\n";
        std::unordered_set<std::string> availExtStrings;
        for (const auto& ext : availableExtensions) {
            availExtStrings.insert(ext.extensionName);
            std::cout << "\t" << ext.extensionName << "\n";
        }
        // Iterate through the required extensions and make sure they are available
        for (const auto str : glfwExtStrings) {
            if (availExtStrings.count(str) < 0) {
                std::cout << "Required Extension not Available: " << str << "\n";
            }
        }

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    /// <summary>
    /// Custom Debug Callback
    /// </summary>
    /// <param name="messageSeverity"> The severity of the message. Can be VERBOSE, INFO, WARNING, ERROR. </param>
    /// <param name="messageType">  The type of the message. Can be GENERAL, VALIDATION, PERFORMANCE. </param>
    /// <param name="pCallbackData"> Contains the actual message, and an array of objects relating to the message. </param>
    /// <param name="pUserData"> Allows you to pass your own data to the function. Specified in callback setup. </param>
    /// <returns> Whether or not to abort the function that trigger this callback. Should always be VK_FALSE. </returns>
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    /// <summary>
    /// If validation is enabled, sets up createInfo struct for our custom callback, then invokes
    /// 'CreateDebugUtilsMessengerEXT' to load the Vulkan creation function and run it.
    /// </summary>
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger.");
        }
    }

    /// <summary>
    /// Populates passed debug messenger create info. Used in creating the standard debug messenger,
    /// and also used as the 'pNext' of the instance create info in order to log instance creation/destruction.
    /// </summary>
    /// <param name="createInfo"></param>
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
    }

    /// <summary>
    /// Finds a suitable physical device and stores it in physicalDevice.
    /// </summary>
    void pickPhysicalDevice() {
        // Query physical device count
        uint32_t pdCount = 0;
        vkEnumeratePhysicalDevices(instance, &pdCount, nullptr);
        if (pdCount == 0) {
            throw std::runtime_error("No physical devices!");
        }

        // Query physical devices
        std::vector<VkPhysicalDevice> devices(pdCount);
        vkEnumeratePhysicalDevices(instance, &pdCount, devices.data());

        for (const auto& device : devices)
        {
            if (isPhysicalDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find suitable physical device!");
        }
    }

    /// <summary>
    /// Queries required device features and decides if suitable.
    /// </summary>
    /// <param name="device"></param>
    /// <returns> Whether the device is suitable.</returns>
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device) 
    {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensions(device);

        bool swapchainSupported = false;
        if (extensionsSupported) {
            SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
            swapchainSupported = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapchainSupported;
    }

    /// <summary>
    /// Queries physical device for the indices of required queue families.
    /// </summary>
    /// <param name="device">The physical device to be queried</param>
    /// <returns>A struct containing indices for all required queue families.</returns>
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilyProperties) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
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

    /// <summary>
    /// Creates the proper logical device and stores it in device.
    /// </summary>
    void createLogicalDevice() {
        QueueFamilyIndices indicies = findQueueFamilies(physicalDevice);

        // Create info for each unique queue family
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies =
            {indicies.graphicsFamily.value(), indicies.presentFamily.value()}; // only holds unique values

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indicies.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        // List of required device features
        VkPhysicalDeviceFeatures deviceFeatures{};

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
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // Create the logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        // Get handle for queue
        vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indicies.presentFamily.value(), 0, &presentQueue);
    }

    /// <summary>
    /// Creates a VkSurface and stores it in surface
    /// </summary>
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    /// <summary>
    /// Queries 'device' for support for extensions defines in DEVICE_EXTENSIONS.
    /// </summary>
    /// <param name="device"> The physical device to query the extension support of.</param>
    /// <returns>Whether or not 'device' supports all extensions found in DEVICE_EXTENSIONS</returns>
    bool checkDeviceExtensions(VkPhysicalDevice device) {

        // First get the number of available extensions, create a vec, then get the extensions
        uint32_t availExtensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &availExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availExtensions(availExtensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &availExtensionCount, availExtensions.data());

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
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        
        return details;
    }

    /// <summary>
    /// Returns the optimal surface format from a list of formats.
    /// (VK_FORMAT_B8G8R8A8_SRGB) and (VK_COLOR_SPACE_SRGB_NONLINEAR_KHR), otherwise just the first of the list.
    /// COULD rank each format and choose highest.
    /// </summary>
    /// <param name="formats"> The list to choose from. </param>
    /// <returns> The optimal surface format from the list. </returns>
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

/// <summary>
/// Returns the optimal present mode from a list of modes.
/// (Chooses mailbox, otherwise returns FIFO, which is guarenteed to be supported.)
/// </summary>
/// <param name="modes"> The list of modes to choose from. </param>
/// <returns> The optimal present mode in 'modes'. </returns>
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> modes) {
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

/// <summary>
/// Returns the optimal Extent given the capabilities.
/// </summary>
/// <param name="capabilities"> The capabilities which to base the optimal extent. </param>
/// <returns> The optimal extent given capabilities. </returns>
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

    // We should just use the current extent, unless it takes a special value
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    // If it is a special value, then use the width/height in pixels, clamped to be within the capabilities.
    else {
        int width, height;

        // Use this function to get size in PIXELS (the WIDTH and HEIGHT properties are in screen space, not pixels).
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Clamp within capabilities
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

/// <summary>
/// Creates a swapchain with optimal parameters, and stores it in 'swapchain'.
/// </summary>
void createSwapchain() {
    // Query the support, and choose optimal parameters
    SwapchainSupportDetails details = querySwapchainSupport(physicalDevice);

    VkExtent2D extent = chooseSwapExtent(details.capabilities);
    VkPresentModeKHR mode = chooseSwapPresentMode(details.presentModes);
    VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(details.formats);

    // Determine the minimum image count
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    // Fill the create info
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.imageExtent = extent;
    createInfo.imageFormat = format.format;
    createInfo.presentMode = mode;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.minImageCount = imageCount;
    createInfo.imageArrayLayers = 1; // 1 unless stereoscopic
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // images used directly for rendering
    createInfo.surface = surface;

    // Get the queue families, and determine whether concurrent image sharing is necessary
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    // Queue families are different
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    // Queue families are the same
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create the swapchain
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain!");
    }

    // Retrieve handles to the swapchain images
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    swapchainFormat = format.format;
    swapchainExtent = extent;
}

void createImageViews() {
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Image View!");
        }
    }
}
    
};

/// <summary>
/// Entry point of our application. Creates the app, and runs it while catching any exceptions.
/// </summary>
/// <returns> EXIT_FAILURE if an exception is thrown, otherwise EXIT_SUCCESS. </returns>
int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}