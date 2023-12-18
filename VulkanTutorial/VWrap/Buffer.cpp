#include "Buffer.h"

namespace VWrap {


    std::shared_ptr<Buffer> Buffer::Create(std::shared_ptr<Device> device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties) {

        auto ret = std::make_shared<Buffer>();
        ret->m_device_ptr = device;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = usage;

        QueueFamilyIndices indices = device->getPhysicalDevicePtr()->FindQueueFamilies();

        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

        bufferInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[2] = { indices.graphicsFamily.value(), indices.transferFamily.value() };
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices;

        bufferInfo.size = size;

        if (vkCreateBuffer(device->getHandle(), &bufferInfo, nullptr, &ret->m_buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device->getHandle(), ret->m_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->getPhysicalDevicePtr()->FindMemoryType(memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device->getHandle(), &allocInfo, nullptr, &ret->m_buffer_memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device->getHandle(), ret->m_buffer, ret->m_buffer_memory, 0);

        return ret;
    }

    std::shared_ptr<Buffer> Buffer::CreateStaging(std::shared_ptr<Device> device,
        VkDeviceSize size) {
        return Create(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    void Buffer::CopyFromBuffer(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src, VkDeviceSize size) {
        auto command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(command_pool);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        copyRegion.dstOffset = 0;
        copyRegion.srcOffset = 0;

        vkCmdCopyBuffer(command_buffer->GetHandle(), src->GetHandle(), m_buffer, 1, &copyRegion);

        VWrap::CommandBuffer::EndSingleTimeCommands(command_buffer);
    }

	Buffer::~Buffer() {
		if (m_buffer != VK_NULL_HANDLE)
			vkDestroyBuffer(m_device_ptr->getHandle(), m_buffer, nullptr);
		if (m_buffer_memory != VK_NULL_HANDLE)
			vkFreeMemory(m_device_ptr->getHandle(), m_buffer_memory, nullptr);
	}
}