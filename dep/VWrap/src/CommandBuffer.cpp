#include "CommandBuffer.h"

namespace VWrap {

	std::shared_ptr<CommandBuffer> CommandBuffer::Create(std::shared_ptr<CommandPool> command_pool, VkCommandBufferLevel level) {
		auto ret = std::make_shared<CommandBuffer>();
		ret->m_command_pool = command_pool;

		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = command_pool->Get();
		info.level = level;
		info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(command_pool->GetDevice()->Get(), &info, &ret->m_command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		return ret;
	}

	std::shared_ptr<CommandBuffer> CommandBuffer::BeginSingleTimeCommands(std::shared_ptr<CommandPool> command_pool) {
		auto ret = Create(command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(ret->m_command_buffer, &info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		return ret;
	}

	void CommandBuffer::EndSingleTimeCommands(std::shared_ptr<CommandBuffer> command_buffer) {
		if (vkEndCommandBuffer(command_buffer->m_command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}

		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &command_buffer->m_command_buffer;

		if (vkQueueSubmit(command_buffer->GetCommandPool()->GetQueue()->Get(), 1, &info, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit command buffer!");
		}

		vkQueueWaitIdle(command_buffer->GetCommandPool()->GetQueue()->Get());
	}
	
	void CommandBuffer::Begin(std::shared_ptr<CommandBuffer> command_buffer) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(command_buffer->Get(), &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer.");
		}
	}
	
	void CommandBuffer::CmdBeginRenderPass(std::shared_ptr<CommandBuffer> command_buffer, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<Framebuffer> framebuffer) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.framebuffer = framebuffer->Get();
		renderPassInfo.renderPass = render_pass->Get();
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = framebuffer->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(command_buffer->Get(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::UploadTextureToImage(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Allocator> allocator, std::shared_ptr<Image>& dst_image, const char* file_name)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(file_name, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("Failed to load images!");
		}

		auto staging_buffer = Buffer::CreateStaging(allocator, imageSize);

		void* data;
		vmaMapMemory(allocator->Get(), staging_buffer->GetAllocation(), &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vmaUnmapMemory(allocator->Get(), staging_buffer->GetAllocation());
		stbi_image_free(pixels);

		VWrap::ImageCreateInfo info{};
		info.width = static_cast<uint32_t>(texWidth);
		info.height = static_cast<uint32_t>(texHeight);
		info.format = VK_FORMAT_R8G8B8A8_SRGB;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		info.mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		dst_image = Image::Create(allocator, info);

		CommandBuffer::TransitionLayout(command_pool, dst_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CommandBuffer::CopyBufferToImage(command_pool, staging_buffer, dst_image, texWidth, texHeight);
		CommandBuffer::GenerateMipmaps(command_pool, dst_image, texWidth, texHeight);
	}

	void CommandBuffer::CopyBufferToImage(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, std::shared_ptr<Image> dst_image, uint32_t width, uint32_t height) {
		auto command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(command_pool);

		VkBufferImageCopy copy{};
		copy.bufferOffset = 0;
		copy.imageOffset = { 0, 0,0 };
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;

		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(command_buffer->Get(), src_buffer->Get(), dst_image->Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		VWrap::CommandBuffer::EndSingleTimeCommands(command_buffer);
	}

	void CommandBuffer::GenerateMipmaps(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Image> image, int32_t width, int32_t height)
	{
		auto format = image->GetFormat();
		auto mip_levels = image->GetMipLevels();
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(image->GetAllocator()->GetDevice()->GetPhysicalDevice()->Get(), format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		auto command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(command_pool);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image->Get();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;
		for (uint32_t i = 1; i < mip_levels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				command_buffer->Get(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				command_buffer->Get(),
				image->Get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image->Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				command_buffer->Get(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mip_levels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer->Get(),
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		CommandBuffer::EndSingleTimeCommands(command_buffer);
	}

	void CommandBuffer::CopyBuffer(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, std::shared_ptr<Buffer> dst_buffer, VkDeviceSize size)
	{
		auto command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(command_pool);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;

		vkCmdCopyBuffer(command_buffer->Get(), src_buffer->Get(), dst_buffer->Get(), 1, &copyRegion);

		VWrap::CommandBuffer::EndSingleTimeCommands(command_buffer);
	}

	void CommandBuffer::TransitionLayout(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Image> image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.newLayout = new_layout;
		barrier.oldLayout = old_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image->Get();
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = image->GetMipLevels();

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		auto graphics_command_buffer = VWrap::CommandBuffer::BeginSingleTimeCommands(command_pool);
		vkCmdPipelineBarrier(
			graphics_command_buffer->Get(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		VWrap::CommandBuffer::EndSingleTimeCommands(graphics_command_buffer);
	}
}