#include "DescriptorSetLayout.h"

namespace VWrap {

	std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::CreateForMeshRasterizer(std::shared_ptr<Device> device) {
		auto ret = std::make_shared<DescriptorSetLayout>();
		ret->m_device = device;

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional - relevant for image sampling descriptors

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional - relevant for image sampling descriptors

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device->Get(), &layoutInfo, nullptr, &ret->m_layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

		return ret;
	 }

    std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::CreateForTracer(std::shared_ptr<Device> device)
    {
        auto ret = std::make_shared<DescriptorSetLayout>();
        ret->m_device = device;

        VkDescriptorSetLayoutBinding sampled_image_binding{};
        sampled_image_binding.binding = 0;
        sampled_image_binding.descriptorCount = 1;
        sampled_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampled_image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        //sampled_image_binding.pImmutableSamplers = nullptr; // Optional - relevant for image sampling descriptors



        std::array<VkDescriptorSetLayoutBinding, 1> bindings = { sampled_image_binding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device->Get(), &layoutInfo, nullptr, &ret->m_layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        return ret;
    }


	DescriptorSetLayout::~DescriptorSetLayout() {
        if (m_layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(m_device->Get(), m_layout, nullptr);
	}
}