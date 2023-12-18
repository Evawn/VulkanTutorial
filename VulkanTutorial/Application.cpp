#include "Application.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void HelloTriangleApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    
    m_glfw_window = std::make_shared<GLFWwindow*>(glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr));

    glfwSetWindowUserPointer(m_glfw_window.get()[0], this);
    glfwSetFramebufferSizeCallback(m_glfw_window.get()[0], glfw_FramebufferResizeCallback);
}

void HelloTriangleApplication::glfw_FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->resized = true;
}

void HelloTriangleApplication::initVulkan() {
    m_instance = VWrap::Instance::Create(enableValidationLayers);
    m_surface = VWrap::Surface::Create(m_instance, m_glfw_window);
    m_physical_device = VWrap::PhysicalDevice::Pick(m_instance, m_surface);
    m_device = VWrap::Device::Create(m_physical_device, enableValidationLayers);

    // Get handle for queue
    VWrap::QueueFamilyIndices indices = m_physical_device->FindQueueFamilies();

    m_graphics_queue = VWrap::Queue::Create(m_device, indices.graphicsFamily.value());
    m_present_queue = VWrap::Queue::Create(m_device, indices.presentFamily.value());
    m_transfer_queue = VWrap::Queue::Create(m_device, indices.transferFamily.value());

    m_swapchain = VWrap::Swapchain::Create(m_device, m_surface);
    CreateSwapchainImageViews();

    m_render_pass = VWrap::RenderPass::Create(m_device, m_swapchain->getFormat());
    m_descriptor_set_layout = VWrap::DescriptorSetLayout::Create(m_device);
    m_pipeline = VWrap::Pipeline::Create(m_device, m_render_pass, m_descriptor_set_layout, m_swapchain->getExtent());

    CreateDepthResources();
    CreateFramebuffers();

    m_graphics_command_pool = VWrap::CommandPool::Create(m_device, m_graphics_queue);
    m_transfer_command_pool = VWrap::CommandPool::Create(m_device, m_transfer_queue);

    
    m_sampler = VWrap::Sampler::Create(m_device);
    m_texture_image = VWrap::Image::Texture2DFromFile(m_device, m_graphics_command_pool, TEXTURE_PATH.data());
    m_texture_image_view = VWrap::ImageView::Create(m_device, m_texture_image);

    LoadModel();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();

    m_descriptor_pool = VWrap::DescriptorPool::Create(m_device, MAX_FRAMES_IN_FLIGHT);
    std::vector<std::shared_ptr<VWrap::DescriptorSetLayout>> layouts(static_cast<size_t>(MAX_FRAMES_IN_FLIGHT), m_descriptor_set_layout);
    m_descriptor_sets = VWrap::DescriptorSet::CreateMany(m_descriptor_pool, layouts);

    UpdateDescriptorSets();
    CreateRenderCommandBuffers();
    CreateRenderSyncObjects();
}

void HelloTriangleApplication::mainLoop() {
    while (!glfwWindowShouldClose(m_glfw_window.get()[0])) {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(m_device->getHandle());
}

void HelloTriangleApplication::drawFrame() {
    VkFence fences[] = { m_in_flight_fences[currentFrame]->GetHandle() };
    vkWaitForFences(m_device->getHandle(), 1, fences, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device->getHandle(), m_swapchain->getHandle(), UINT64_MAX, m_image_available_semaphores[currentFrame]->GetHandle(), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetCommandBuffer(m_render_command_buffers[currentFrame]->GetHandle(), 0);
    recordCommandBuffer(m_render_command_buffers[currentFrame]->GetHandle(), imageIndex);

    updateUniformBuffer(currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_image_available_semaphores[currentFrame]->GetHandle() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;

    std::array<VkCommandBuffer, 1> commandBuffers = { m_render_command_buffers[currentFrame]->GetHandle() };
    submitInfo.pCommandBuffers = commandBuffers.data();

    VkSemaphore signalSemaphores[] = { m_render_finished_semaphores[currentFrame]->GetHandle()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_device->getHandle(), 1, fences);

    if (vkQueueSubmit(m_graphics_queue->GetHandle(), 1, &submitInfo, m_in_flight_fences[currentFrame]->GetHandle()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit to graphics queue!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { m_swapchain->getHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(m_present_queue->GetHandle(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
        resized = false;
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t frame) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_swapchain->getExtent().width / (float)m_swapchain->getExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_uniform_buffers_mapped[currentFrame], &ubo, sizeof(ubo));
}

void HelloTriangleApplication::cleanup() {
    glfwDestroyWindow(m_glfw_window.get()[0]);
    glfwTerminate();
}


void HelloTriangleApplication::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_glfw_window.get()[0], &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_glfw_window.get()[0], &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_device->getHandle());

    for (auto framebuffer : m_framebuffers)
        framebuffer.reset();
    for (auto imageView : m_swapchain_image_views)
        imageView.reset();
    m_swapchain.reset();
    m_depth_image.reset();
    m_depth_image_view.reset();

    m_swapchain = VWrap::Swapchain::Create(m_device, m_surface);
    CreateSwapchainImageViews();
    CreateDepthResources();
    CreateFramebuffers();
}

/// <summary>
/// Populates 'swapchainImageViews' with VkImageViews corresponding to the VkImages in 'swapchainImages'.
/// </summary>
void HelloTriangleApplication::CreateSwapchainImageViews() {
    m_swapchain_image_views.resize(m_swapchain->getImageHandles().size());
    for (size_t i = 0; i < m_swapchain->getImageHandles().size(); i++) 
        m_swapchain_image_views[i] = VWrap::ImageView::Create(m_device, m_swapchain->getImageHandles()[i], m_swapchain->getFormat());
}

/// <summary>
/// Creates a framebuffer for every image view and stores them in 'framebuffers'
/// </summary>
void HelloTriangleApplication::CreateFramebuffers() {
    m_framebuffers.resize(m_swapchain_image_views.size());
    for (uint32_t i = 0; i < m_swapchain_image_views.size(); i++) 
        m_framebuffers[i] = VWrap::Framebuffer::Create2D(m_device, m_render_pass, m_swapchain_image_views[i], m_depth_image_view , m_swapchain->getExtent());
}

/// <summary>
/// Creates a command buffer and stores it in 'commandBuffer'
/// </summary>
void HelloTriangleApplication::CreateRenderCommandBuffers() {
    m_render_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        m_render_command_buffers[i] = VWrap::CommandBuffer::Create(m_graphics_command_pool);
}

/// <summary>
/// Creates fences and semaphores necessary for drawing and presentation synchronization
/// </summary>
void HelloTriangleApplication::CreateRenderSyncObjects() {
    m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_image_available_semaphores[i] = VWrap::Semaphore::Create(m_device);
        m_render_finished_semaphores[i] = VWrap::Semaphore::Create(m_device);
        m_in_flight_fences[i] = VWrap::Fence::Create(m_device);
    }
}

/// <summary>
/// Creates and allocates memory for a vertex buffer using data from 'vertices', and stores the buffer
/// in 'vertexBuffer' and its memory in 'vertexBufferMemory'
/// </summary>
void HelloTriangleApplication::CreateVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto staging_buffer = VWrap::Buffer::CreateStaging(m_device, 
        bufferSize);

    void* data;
    vkMapMemory(m_device->getHandle(), staging_buffer->GetMemory(), 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device->getHandle(), staging_buffer->GetMemory());

    m_vertex_buffer = VWrap::Buffer::Create(m_device,
        		bufferSize,
        		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_vertex_buffer->CopyFromBuffer(m_transfer_command_pool, staging_buffer, bufferSize);
}

void HelloTriangleApplication::CreateIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto staging_buffer = VWrap::Buffer::CreateStaging(m_device, bufferSize);

    void* data;
    vkMapMemory(m_device->getHandle(), staging_buffer->GetMemory(), 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device->getHandle(), staging_buffer->GetMemory());

    m_index_buffer = VWrap::Buffer::Create(m_device,
        		bufferSize,
        		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_index_buffer->CopyFromBuffer(m_transfer_command_pool, staging_buffer, bufferSize);
}

void HelloTriangleApplication::CreateUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_uniform_buffers[i] = VWrap::Buffer::Create(m_device,
            			bufferSize,
            			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(m_device->getHandle(), m_uniform_buffers[i]->GetMemory(), 0, bufferSize, 0, &m_uniform_buffers_mapped[i]);
    }
}

void HelloTriangleApplication::UpdateDescriptorSets() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniform_buffers[i]->GetHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_texture_image_view->GetHandle();
        imageInfo.sampler = m_sampler->GetHandle();

        // array of descriptor writes:
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstSet = m_descriptor_sets[i]->GetHandle();
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstSet = m_descriptor_sets[i]->GetHandle();
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        vkUpdateDescriptorSets(m_device->getHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void HelloTriangleApplication::CreateDepthResources()
{
    m_depth_image = VWrap::Image::CreateDepthImage(m_device, m_graphics_command_pool, m_swapchain->getExtent());
    m_depth_image_view = VWrap::ImageView::Create(m_device, m_depth_image, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void HelloTriangleApplication::LoadModel()
{
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
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
    std::cout << "Finished loading" << std::endl;
}


/// <summary>
/// Records a simple drawing command to the framebuffer at imageIndex
/// </summary>
/// <param name="commnadBuffer"> The command buffer to record to. </param>
/// <param name="imageIndex"> The index of the framebuffer to draw to. </param>
void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.framebuffer = m_framebuffers[imageIndex]->GetHandle();
    renderPassInfo.renderPass = m_render_pass->GetHandle();
    renderPassInfo.renderArea.offset = { 0,0 };
    renderPassInfo.renderArea.extent = m_swapchain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetHandle());

    std::array<VkDescriptorSet, 1> descriptorSets = { m_descriptor_sets[currentFrame]->GetHandle() };
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetLayoutHandle(), 0, 1, descriptorSets.data(), 0, nullptr);

    // TODO: have dynamic state and give scissor/viewport right here
    //vkCmdSetViewport()
    //vkCmdSetScissor()

    VkBuffer vertexBuffers[] = { m_vertex_buffer->GetHandle() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_index_buffer->GetHandle(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to end command buffer recording!");
    }
}