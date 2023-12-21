#include "DescriptorPool.h"

namespace VWrap {

	std::shared_ptr<DescriptorPool> DescriptorPool::Create(std::shared_ptr<Device> device, uint32_t num_sets) {
		auto ret = std::make_shared<DescriptorPool>();
		ret->m_device = device;

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].descriptorCount = num_sets;
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[1].descriptorCount = num_sets;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        //createInfo.flags = 0;
        createInfo.maxSets = num_sets;
        createInfo.pPoolSizes = poolSizes.data();
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());

        if (vkCreateDescriptorPool(device->GetHandle(), &createInfo, nullptr, &ret->m_descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

		return ret;
	}

	DescriptorPool::~DescriptorPool() {
		if (m_descriptor_pool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(m_device->GetHandle(), m_descriptor_pool, nullptr);
	}
}