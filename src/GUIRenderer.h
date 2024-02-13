#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <memory>
#include "DescriptorPool.h"
#include "RenderPass.h"
#include "Device.h"
#include "Queue.h"
#include "CommandBuffer.h"

/// <summary>
/// Wrapper for ImGui control. Defines GUI and render it.
/// </summary>
class GUIRenderer
{
private:

	/// <summary>
	/// Descriptor pool for ImGui.
	/// </summary>
	std::shared_ptr<VWrap::DescriptorPool> m_imgui_descriptor_pool;

	/// <summary>
	/// The DPI scale.
	/// </summary>
	float m_dpi_scale = 1.0f;

public:

	/// <summary>
	/// Creates GUIRenderer, instantiates ImGui context.
	/// </summary>
	static std::shared_ptr<GUIRenderer> Create(std::shared_ptr<VWrap::Device> device);

	/// <summary>
	/// Records to the command buffer ImGui draw commands.
	/// </summary>
	void CmdDraw(std::shared_ptr<VWrap::CommandBuffer> command_buffer, float time, float fps, float& sensitivity, float& speed);

	void BeginFrame();

	/// <summary>
	/// Returns descriptor pool for ImGui.
	/// </summary>
	std::shared_ptr<VWrap::DescriptorPool> GetDescriptorPool() const {
		return m_imgui_descriptor_pool;
	}

	/// <summary>
	/// Sets DPI scale for ImGui.
	/// </summary>
	void SetDpiScale(float dpi_scale) {
		ImGuiIO& io = ImGui::GetIO();
		m_dpi_scale = dpi_scale;
		io.FontGlobalScale = dpi_scale;
	}
};

