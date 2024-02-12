#include "DescriptorPool.h"

namespace VWrap {

	std::shared_ptr<DescriptorPool> DescriptorPool::CreateForRasterizer(std::shared_ptr<Device> device, uint32_t max_sets) {

        std::vector<VkDescriptorPoolSize> poolSizes(2);
        poolSizes[0].descriptorCount = max_sets;
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[1].descriptorCount = max_sets;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		return Create(device, poolSizes, max_sets, 0);
	}

    std::shared_ptr<DescriptorPool> DescriptorPool::CreateForTracer(std::shared_ptr<Device> device, uint32_t max_sets)
    {
        std::vector<VkDescriptorPoolSize> poolSizes(1);
        poolSizes[0].descriptorCount = max_sets;
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        return Create(device, poolSizes, max_sets, 0);
    }

    std::shared_ptr<DescriptorPool> DescriptorPool::Create(std::shared_ptr<Device> device, std::vector<VkDescriptorPoolSize> pool_sizes, uint32_t max_sets, VkDescriptorPoolCreateFlags flags)
    {
        auto ret = std::make_shared<DescriptorPool>();
        ret->m_device = device;

        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.flags = flags;
        createInfo.maxSets =max_sets;
        createInfo.pPoolSizes = pool_sizes.data();
        createInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());

        if (vkCreateDescriptorPool(device->Get(), &createInfo, nullptr, &ret->m_descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        return ret;
    }

	DescriptorPool::~DescriptorPool() {
		if (m_descriptor_pool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(m_device->Get(), m_descriptor_pool, nullptr);
	}
}