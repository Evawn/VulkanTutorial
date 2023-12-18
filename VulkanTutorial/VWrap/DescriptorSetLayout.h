#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include <array>

namespace VWrap {

	class DescriptorSetLayout {
	private:
			VkDescriptorSetLayout m_layout;
			std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<DescriptorSetLayout> Create(std::shared_ptr<Device> device);

		VkDescriptorSetLayout GetHandle() const { return m_layout; }

		~DescriptorSetLayout();

	};
}