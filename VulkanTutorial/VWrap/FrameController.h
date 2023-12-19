#pragma once
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Image.h"
#include "ImageView.h"
#include "Semaphore.h"
#include "Fence.h"
#include "Device.h"
#include "Surface.h"
#include <functional>

namespace VWrap {

	class FrameController {
	private:
		uint32_t MAX_FRAMES_IN_FLIGHT;
		bool resized = false;

		std::shared_ptr<Device> m_device_ptr;
		std::shared_ptr<Swapchain> m_swapchain;
		std::shared_ptr<Surface> m_surface;

		std::vector<std::shared_ptr<CommandBuffer>> m_command_buffers;

		std::vector<std::shared_ptr<ImageView>> m_image_views;
		std::vector<std::shared_ptr<Semaphore>> m_image_available_semaphores, m_render_finished_semaphores;
		std::vector<std::shared_ptr<Fence>> m_in_flight_fences;

		std::shared_ptr<CommandPool> m_graphics_command_pool;
		std::shared_ptr<Queue> m_present_queue;

		uint32_t m_current_frame = 0;
		uint32_t m_image_index = 0;

		std::function<void()> m_resize_callback;

		void CreateImageViews();

		void CreateCommandBuffers();

		void CreateSyncObjects();

	public:

		static std::shared_ptr<FrameController> Create(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface, std::shared_ptr<CommandPool> graphics_pool, std::shared_ptr<Queue> present_queue, uint32_t max_frames);

		std::vector<std::shared_ptr<CommandBuffer>> GetCommandBuffers() { return m_command_buffers; }
		std::vector<std::shared_ptr<Semaphore>> GetImageAvailableSemaphores() { return m_image_available_semaphores; }
		std::vector<std::shared_ptr<Semaphore>> GetRenderFinishedSemaphores() { return m_render_finished_semaphores; }
		std::vector<std::shared_ptr<Fence>> GetInFlightFences() { return m_in_flight_fences; }
		std::vector<std::shared_ptr<ImageView>> GetImageViews() { return m_image_views; }
		std::shared_ptr<Swapchain> GetSwapchain() { return m_swapchain; }

		uint32_t GetCurrentFrame() { return m_current_frame; }
		uint32_t GetImageIndex() { return m_image_index; }

		std::shared_ptr<CommandBuffer> GetCurrentCommandBuffer() { return m_command_buffers[m_current_frame]; }

		void AcquireNext();

		void Render();

		void RecreateSwapchain();

		void SetResizeCallback(std::function<void()> lambda) { m_resize_callback = lambda; }

		void SetResized(bool resized) { this->resized = resized; }
	};
}