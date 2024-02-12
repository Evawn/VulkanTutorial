#include "OctreeTracer.h"

std::shared_ptr<OctreeTracer> OctreeTracer::Create(std::shared_ptr<VWrap::Allocator> allocator, std::shared_ptr<VWrap::Device> device, std::shared_ptr<VWrap::RenderPass> render_pass, std::shared_ptr<VWrap::CommandPool> graphics_pool, VkExtent2D extent, uint32_t num_frames) {
	auto ret = std::make_shared<OctreeTracer>();
	ret->m_device = device;
	ret->m_allocator = allocator;
	ret->m_extent = extent;
	ret->m_graphics_pool = graphics_pool;

	ret->m_descriptor_set_layout = VWrap::DescriptorSetLayout::CreateForTracer(device);
	ret->m_pipeline = VWrap::Pipeline::CreateTracer(device, render_pass, ret->m_descriptor_set_layout, extent);

	ret->m_sampler = VWrap::Sampler::Create(device);
	VWrap::CommandBuffer::CreateAndFillBrickTexture(graphics_pool, allocator, ret->m_brick_texture, 32);
	ret->m_brick_texture_view = VWrap::ImageView::Create3D(device, ret->m_brick_texture);

	ret->m_descriptor_pool = VWrap::DescriptorPool::CreateForTracer(device, num_frames);
	std::vector<std::shared_ptr<VWrap::DescriptorSetLayout>> layouts(static_cast<size_t>(num_frames), ret->m_descriptor_set_layout);
	ret->m_descriptor_sets = VWrap::DescriptorSet::CreateMany(ret->m_descriptor_pool, layouts);

	ret->UpdateDescriptorSets();

	return ret;
}

void OctreeTracer::UpdateDescriptorSets()
{
	for (size_t i = 0; i < m_descriptor_sets.size(); i++) {

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = m_brick_texture_view->Get();
		//imageInfo.sampler = m_sampler->Get();

		// array of descriptor writes:
		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstSet = m_descriptor_sets[i]->Get();
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].pImageInfo = &imageInfo;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		vkUpdateDescriptorSets(m_device->Get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void OctreeTracer::CmdDraw(std::shared_ptr<VWrap::CommandBuffer> command_buffer, uint32_t frame, std::shared_ptr<Camera> camera)
{
	auto vk_command_buffer = command_buffer->Get();
	vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->Get());

	std::array<VkDescriptorSet, 1> descriptorSets = { m_descriptor_sets[frame]->Get() };
	vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetLayout(), 0, 1, descriptorSets.data(), 0, nullptr);


	VkViewport viewport{};
	viewport.height = (float)m_extent.height;
	viewport.width = (float)m_extent.width;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0;
	viewport.y = 0;

	vkCmdSetViewport(vk_command_buffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = m_extent;
	scissor.offset = { 0,0 };

	vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);

	VWrap::PushConstantBlock PCB;
	PCB.NDCtoWorld = camera->GetNDCtoWorldMatrix();
	PCB.cameraPos = camera->GetPosition();

	// Populate pushConstants with the necessary data
	vkCmdPushConstants(
		vk_command_buffer,
		m_pipeline->GetLayout(), // The pipeline layout used for the push constants
		VK_SHADER_STAGE_FRAGMENT_BIT, // Shader stage the push constants will be used in
		0, // Offset of the push constants to update
		sizeof(VWrap::PushConstantBlock), // Size of the push constants to update
		&PCB // Pointer to the data to copy
	);

	vkCmdDraw(vk_command_buffer, 4, 1, 0, 0);
}
