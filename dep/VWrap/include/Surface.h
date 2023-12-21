#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <memory>
#include "Instance.h"

namespace VWrap {

	class Surface {

	private:

		VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
		std::shared_ptr<Instance> m_instance_ptr; 
		std::shared_ptr<GLFWwindow*> m_window_ptr;

	public:

		static std::shared_ptr<Surface> Create(std::shared_ptr<Instance> instance, std::shared_ptr<GLFWwindow*> window) {
			auto ret = std::make_shared<Surface>();
			ret->m_instance_ptr = instance;
			ret->m_window_ptr = window;

			if (glfwCreateWindowSurface(instance->getHandle(), *window, nullptr, &ret->m_surface) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create window surface!");
			}
			return ret;
		}

		~Surface() {
			if (m_surface != VK_NULL_HANDLE) {
				vkDestroySurfaceKHR(m_instance_ptr->getHandle(), m_surface, nullptr);
			}
		}

		VkSurfaceKHR getHandle() const {
			return m_surface;
		}

		std::shared_ptr<Instance> getInstancePtr() const {
			return m_instance_ptr;
		}
		
		std::shared_ptr<GLFWwindow*> getWindowPtr() const {
			return m_window_ptr;
		}

	};

}