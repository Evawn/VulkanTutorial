#pragma once
#include "vulkan/vulkan.h"
#include <memory>
#include "Device.h"
#include "CommandPool.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Image.h"
#include "Buffer.h"
#include "stb_image.h"

namespace VWrap {

	/// <summary>
	/// Represents a command buffer. Contains static methods for common command buffer operations.
	/// </summary>
	class CommandBuffer
	{
	private:

		/// <summary>
		/// The underlying vulkan command buffer handle.
		/// </summary>
		VkCommandBuffer m_command_buffer{ VK_NULL_HANDLE };

		/// <summary>
		/// The command pool that this command buffer was allocated from.
		/// </summary>
		std::shared_ptr<CommandPool> m_command_pool;

	public:

		/// <summary>
		/// Allocates a new command buffer from the given pool.
		/// </summary>
		/// <param name="command_pool">The command pool to allocate the command buffer from.</param>
		/// <param name="level">The level of the command buffer.</param>
		/// <returns>A shared pointer to the command buffer.</returns>
		static std::shared_ptr<CommandBuffer> Create(std::shared_ptr<CommandPool> command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		/// <summary>
		/// Allocates a single-use command buffer from the given command pool, and begins recording.
		/// </summary>
		/// <param name="command_pool">The command pool to allocate the command buffer from.</param>
		/// <returns>A shared pointer to the command buffer.</returns>
		static std::shared_ptr<CommandBuffer> BeginSingleTimeCommands(std::shared_ptr<CommandPool> command_pool);

		/// <summary>
		/// Ends recording of the given command buffer, and submits it to the given queue.
		/// </summary>
		static void EndSingleTimeCommands(std::shared_ptr<CommandBuffer> command_buffer);

		static void Begin(std::shared_ptr<CommandBuffer> command_buffer);

		/// <summary>
		/// Records a command to begin the given render pass.
		/// </summary>
		static void CmdBeginRenderPass(std::shared_ptr<CommandBuffer> command_buffer, std::shared_ptr<RenderPass> render_pass, std::shared_ptr<Framebuffer> framebuffer);

		/// <summary>
		/// Creates an image at the dst_image handle, and uploads the given texture to it.
		/// </summary>
		static void UploadTextureToImage(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Allocator> allocator, std::shared_ptr<Image>& dst_image, const char* file_name);

		/// <summary>
		/// Copies the data from the source buffer to this buffer
		/// </summary>
		static void CopyBufferToImage(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, std::shared_ptr<Image> dst_image, uint32_t width, uint32_t height);

		/// <summary>
		/// Builds the mipmaps for the given image.
		/// </summary>
		static void GenerateMipmaps(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Image> image, int32_t tex_width, int32_t tex_height);

		/// <summary>
		/// Copies the data from the source buffer to the destination buffer
		/// </summary>
		static void CopyBuffer(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Buffer> src_buffer, std::shared_ptr<Buffer> dst_buffer, VkDeviceSize size);

		/// <summary>
		/// Submits a pipeline barrier to transition the image layout
		/// </summary>
		static void TransitionLayout(std::shared_ptr<CommandPool> command_pool, std::shared_ptr<Image> image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

		/// <summary>
		/// Gets the underlying vulkan command buffer handle.
		/// </summary>
		VkCommandBuffer Get() const { return m_command_buffer; }

		/// <summary>
		/// Gets the command pool that this command buffer was allocated from.
		/// </summary>
		std::shared_ptr<CommandPool> GetCommandPool() const { return m_command_pool; }
	};
}