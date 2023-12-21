#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"

namespace VWrap {
	class Sampler
	{
	private:

		VkSampler m_sampler{ VK_NULL_HANDLE };

		std::shared_ptr<Device> m_device_ptr;

	public:

		static std::shared_ptr<Sampler> Create(std::shared_ptr<Device> device);

		VkSampler GetHandle() const { return m_sampler; }

		~Sampler();
	};

}
