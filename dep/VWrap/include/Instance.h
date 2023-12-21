#ifndef INSTANCE_H
#define INSTANCE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <string>
#include <memory>


namespace VWrap {
	const std::vector<const char*> VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DEVICE_EXTENSIONS = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	/// <summary>
/// Proxy function that loads the extension function vkCreateDebugUtilsMessengerEXT and then runs it.
/// </summary>
/// <param name="instance"> The instance to look up the address of the func. </param>
/// <param name="pCreateInfo"> Our create struct. </param>
/// <param name="pAllocator"> Custom Allocator. </param>
/// <param name="pDebugMessenger">  Where to store the callback handle. </param>
/// <returns> Whether the function was successfully loaded and ran. </returns>
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
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
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
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
/// Populates passed debug messenger create info. Used in creating the standard debug messenger,
/// and also used as the 'pNext' of the instance create info in order to log instance creation/destruction.
/// </summary>
/// <param name="createInfo"></param>
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
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
/// Queries GLFW for required extensions, and queries Vulkan for available extentions.
/// If the GLFW extensions are not available, it will print it out.
/// </summary>
/// <returns> The required extensions, including validation extensions if enabled. </returns>
	static std::vector<const char*> getRequiredExtensions(bool enableValidationLayers) {

		// Get the names of the required instance extensions from GLFW
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Store the names as a vector<string> for later comparison
		std::vector<std::string> glfwExtStrings;
		for (uint32_t i = 0; i < glfwExtensionCount; i++) {
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
/// Queries Vulkan for available validation layers, and compares it with the elements of VALDIATION_LAYERS.
/// </summary>
/// <returns> Whether all elements of VALIDATION_LAYERS are available. </returns>
	static bool checkValidationLayerSupport() {
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

	class Instance {

	public:
		static std::shared_ptr<Instance> Create(bool enableValidationLayers);

		VkInstance getHandle() const { return m_instance; }

		~Instance();
	private:
		VkInstance m_instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };
	};
}
#endif