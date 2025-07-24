#pragma once

#include <cstdint>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

namespace gfx
{
	/// @brief The FramebufferSize struct stores framebuffer size in pixels.
	struct FramebufferSize
	{
		uint32_t width;
		uint32_t height;
	};

	/// @brief The FrameState struct contains per-frame data for the render-backend.
	struct FrameState
	{
		WGPUTexture		swapTexture;
		WGPUTextureView swapTextureView;
	};

	/// @brief The RenderBackend handles graphics API initialization and configuration.
	class RenderBackend
	{
	public:
		RenderBackend(GLFWwindow* pWindow);
		~RenderBackend();

		RenderBackend(RenderBackend const&) = delete;
		RenderBackend& operator=(RenderBackend const&) = delete;

		/// @brief Start rendering a new frame.
		/// @param state FrameState populated on successful frame start.
		/// @return A boolean indicating successful frame start.
		bool newFrame(FrameState& state);

		/// @brief Present the currently acquired frame.
		/// @param state 
		void present(FrameState& state);

		/// @brief Submit recorded command buffers to the GPU, starting work.
		/// @param commandCount 
		/// @param pCommands 
		void submit(size_t commandCount, WGPUCommandBuffer const* pCommands);

		/// @brief Resize the swap surface framebuffer size.
		/// @param size 
		void resizeSwapBuffers(FramebufferSize const& size);

		FramebufferSize		getFramebufferSize() const	{ return m_framebufferSize; }
		bool				hasSRGBFramebuffer() const	{ return m_surfaceInfo.isSRGB; }
		WGPUTextureFormat	getSwapFormat() const		{ return m_surfaceInfo.preferredFormat; }
		WGPUInstance		getInstance() const			{ return m_instance; }
		WGPUSurface			getSurface() const			{ return m_surface; }
		WGPUDevice			getDevice() const			{ return m_device; }
		WGPUQueue			getQueue() const			{ return m_queue; }

	private:
		/// @brief Used to store some info on the currently configured surface.
		struct SurfaceInfo
		{
			bool				isSRGB;
			WGPUTextureFormat	preferredFormat;
			bool				hasMailboxPresent;
			bool				hasImmediatePresent;
			WGPUPresentMode		currentPresentMode;
		};

	private:
		/// @brief Request an adapter based on the given adapter options.
		/// @param instance 
		/// @param pOptions 
		/// @return 
		WGPUAdapter requestAdapter(WGPUInstance instance, WGPURequestAdapterOptions* pOptions) const;

		/// @brief Request a logical device from an adapter.
		/// @param adapter 
		/// @param pDesc 
		/// @return 
		WGPUDevice requestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor* pDesc) const;

		/// @brief Get information on the current surface state.
		/// @param surface 
		/// @param adapter 
		/// @return 
		SurfaceInfo getSurfaceInfo(WGPUSurface surface, WGPUAdapter adapter) const;

		/// @brief Poll device to handle any work remaining on the queue.
		void pollDeviceState();

	private:
		FramebufferSize	m_framebufferSize	= {};
		WGPUInstance	m_instance			= nullptr;
		WGPUSurface		m_surface			= nullptr;
		WGPUAdapter		m_adapter			= nullptr;
		WGPUDevice		m_device			= nullptr;
		WGPUQueue		m_queue				= nullptr;
		SurfaceInfo		m_surfaceInfo		= {};
	};
} // namespace gfx
