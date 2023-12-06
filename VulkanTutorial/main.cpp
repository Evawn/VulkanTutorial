#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
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

        bool isComplete() {
            return graphicsFamily.has_value();
        }
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
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
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
        return indices.isComplete();
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

        // Create info for a single queue family
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indicies.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // List of required device features
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Create info for logical device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Specify device-specific extensions
        createInfo.enabledExtensionCount = 0;
        if (enableValidationLayers) {
            createInfo.enabledExtensionCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
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
    }

    /// <summary>
    /// Creates a VkSurface and stores it in surface
    /// </summary>
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
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