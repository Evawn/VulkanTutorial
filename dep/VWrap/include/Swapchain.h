#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include "PhysicalDevice.h"
#include "Device.h"
#include "Surface.h"
#include <algorithm>


namespace VWrap {
	class Swapchain {

	private:
		VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
		std::shared_ptr<Device> m_device_ptr;
		VkFormat m_swapchain_image_format;
		VkExtent2D m_swapchain_extent;
		std::vector<VkImage> m_swapchain_images;
	

		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> formats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> modes);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);


	public:
		static std::shared_ptr<Swapchain> Create(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface);

		~Swapchain();

		VkSwapchainKHR getHandle() const {
			return m_swapchain;
		}

		std::shared_ptr<Device> getDevicePtr() const {
			return m_device_ptr;
		}

		VkFormat getFormat() const {
			return m_swapchain_image_format;
		}

		VkExtent2D getExtent() const {
			return m_swapchain_extent;
		}

		std::vector<VkImage> getImageHandles() const {
			return m_swapchain_images;
		}

		size_t getImageCount() const {
			return m_swapchain_images.size();
		}

	};
}