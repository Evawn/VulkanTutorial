#include "MeshRasterizer.h"

inline void MeshRasterizer::CreateVertexBuffer() {
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	auto staging_buffer = VWrap::Buffer::CreateStaging(m_allocator, bufferSize);

	void* data;
	vmaMapMemory(m_allocator->Get(), staging_buffer->GetAllocation(), &data);
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vmaUnmapMemory(m_allocator->Get(), staging_buffer->GetAllocation());

	m_vertex_buffer = VWrap::Buffer::Create(m_allocator,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0);

	VWrap::CommandBuffer::CopyBuffer(m_graphics_pool, staging_buffer, m_vertex_buffer, bufferSize);
}

inline void MeshRasterizer::CreateIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	auto staging_buffer = VWrap::Buffer::CreateStaging(m_allocator, bufferSize);

	void* data;
	vmaMapMemory(m_allocator->Get(), staging_buffer->GetAllocation(), &data);
	memcpy(data, m_indices.data(), (size_t)bufferSize);
	vmaUnmapMemory(m_allocator->Get(), staging_buffer->GetAllocation());

	m_index_buffer = VWrap::Buffer::Create(m_allocator,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0);

	VWrap::CommandBuffer::CopyBuffer(m_graphics_pool, staging_buffer, m_index_buffer, bufferSize);
}

inline void MeshRasterizer::CreateUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	auto frames = m_descriptor_sets.size();
	m_uniform_buffers.resize(frames);
	m_uniform_buffers_mapped.resize(frames);

	for (size_t i = 0; i < frames; i++) {
		//VmaAllocationInfo alloc_info{};
		m_uniform_buffers[i] = VWrap::Buffer::CreateMapped(
			m_allocator,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_uniform_buffers_mapped[i]);
	}
}

inline void MeshRasterizer::UpdateDescriptorSets() {
	for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniform_buffers[i]->Get();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_texture_image_view->Get();
		imageInfo.sampler = m_sampler->Get();

		// array of descriptor writes:
		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstSet = m_descriptor_sets[i]->Get();
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstSet = m_descriptor_sets[i]->Get();
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		vkUpdateDescriptorSets(m_device->Get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

inline void MeshRasterizer::LoadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<VWrap::Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			VWrap::Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
				m_vertices.push_back(vertex);
			}

			m_indices.push_back(uniqueVertices[vertex]);
		}
	}
	std::cout << "Finished loading" << std::endl;
}

std::shared_ptr<MeshRasterizer> MeshRasterizer::Create(std::shared_ptr<VWrap::Allocator> allocator, std::shared_ptr<VWrap::Device> device, std::shared_ptr<VWrap::RenderPass> render_pass, std::shared_ptr<VWrap::CommandPool> graphics_pool, VkExtent2D extent, uint32_t num_frames) {
	auto ret = std::make_shared<MeshRasterizer>();
	ret->m_device = device;
	ret->m_allocator = allocator;
	ret->m_extent = extent;
	ret->m_graphics_pool = graphics_pool;

	ret->m_descriptor_set_layout = VWrap::DescriptorSetLayout::Create(device);
	ret->m_pipeline = VWrap::Pipeline::Create(device, render_pass, ret->m_descriptor_set_layout, extent);
	ret->m_sampler = VWrap::Sampler::Create(device);

	VWrap::CommandBuffer::UploadTextureToImage(graphics_pool, allocator, ret->m_texture_image, TEXTURE_PATH.c_str());
	ret->m_texture_image_view = VWrap::ImageView::Create(device, ret->m_texture_image);


	ret->m_descriptor_pool = VWrap::DescriptorPool::Create(device, num_frames);
	std::vector<std::shared_ptr<VWrap::DescriptorSetLayout>> layouts(static_cast<size_t>(num_frames), ret->m_descriptor_set_layout);
	ret->m_descriptor_sets = VWrap::DescriptorSet::CreateMany(ret->m_descriptor_pool, layouts);

	ret->LoadModel();
	ret->CreateVertexBuffer();
	ret->CreateIndexBuffer();
	ret->CreateUniformBuffers();

	ret->UpdateDescriptorSets();

	return ret;
}

void MeshRasterizer::CmdDraw(std::shared_ptr<VWrap::CommandBuffer> command_buffer, uint32_t frame) {
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

	VkBuffer vertexBuffers[] = { m_vertex_buffer->Get() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(vk_command_buffer, m_index_buffer->Get(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(vk_command_buffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}

void MeshRasterizer::UpdateUniformBuffer(uint32_t frame, std::shared_ptr<Camera> camera) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = camera->GetViewMatrix();
	ubo.proj = camera->GetProjectionMatrix();

	//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.proj = glm::perspective(glm::radians(45.0f), (float)m_extent.width / (float)m_extent.height, 0.1f, 10.0f);
	//ubo.proj[1][1] *= -1;

	memcpy(m_uniform_buffers_mapped[frame], &ubo, sizeof(ubo));
}
