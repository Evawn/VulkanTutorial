#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include <vector>

namespace VWrap {
	class DescriptorSet
	{
	private:

		VkDescriptorSet m_descriptor_set{ VK_NULL_HANDLE };
		std::shared_ptr<DescriptorPool> m_descriptor_pool_ptr;
		std::shared_ptr<DescriptorSetLayout> m_layout_ptr;

	public:

		static std::shared_ptr<DescriptorSet> Create(std::shared_ptr<DescriptorPool> descriptor_pool, std::shared_ptr<DescriptorSetLayout> layout);

		static std::vector<std::shared_ptr<DescriptorSet>> CreateMany(std::shared_ptr<DescriptorPool> descriptor_pool, std::vector<std::shared_ptr<DescriptorSetLayout>> layouts);

		VkDescriptorSet GetHandle() const { return m_descriptor_set; }

		std::shared_ptr<DescriptorPool> GetDescriptorPool() const { return m_descriptor_pool_ptr; }

		//~DescriptorSet();
	};

}