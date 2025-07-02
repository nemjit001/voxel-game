#include "render_backend.hpp"

#include <cassert>
#include <stdexcept>
#include <glfw3webgpu.h>
#include <spdlog/spdlog.h>

namespace gfx
{
	static void wgpuErrorCallback(WGPUErrorType type, char const* pMessage, void* pUserData)
	{
		(void)(type);
		(void)(pUserData);
		spdlog::error("[WebGPU] {}", pMessage);
	}

	RenderBackend::RenderBackend(GLFWwindow* pWindow)
	{
		assert(pWindow);

		// Get window framebuffer size
		int w, h;
		glfwGetFramebufferSize(pWindow, &w, &h);
		m_framebufferSize.width = static_cast<uint32_t>(w);
		m_framebufferSize.height = static_cast<uint32_t>(h);

		// Initialize WebGPU instance
		m_instance = wgpuCreateInstance(nullptr);
		if (!m_instance) {
			throw std::runtime_error("WGPU instance create failed");
		}

		// Initialize WebGPU surface
		m_surface = glfwGetWGPUSurface(m_instance, pWindow);
		if (!m_surface) {
			throw std::runtime_error("WGPU surface create failed");
		}

		// Request a WebGPU adapter and device
		WGPURequestAdapterOptions adapterOptions{};
		adapterOptions.nextInChain = nullptr;
		adapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;
		adapterOptions.compatibleSurface = m_surface;

		WGPUDeviceDescriptor deviceDesc{};
		deviceDesc.nextInChain = nullptr;
		deviceDesc.label = "WGPU device";
		deviceDesc.requiredFeatureCount = 0;
		deviceDesc.requiredFeatures = nullptr;
		deviceDesc.requiredLimits = nullptr;
		deviceDesc.defaultQueue.nextInChain = nullptr;
		deviceDesc.defaultQueue.label = "WGPU queue";
		deviceDesc.deviceLostCallback = nullptr;
		deviceDesc.deviceLostUserdata = nullptr;

		m_adapter = requestAdapter(m_instance, &adapterOptions);
		m_device = requestDevice(m_adapter, &deviceDesc);
		if (!m_adapter || !m_device) {
			throw std::runtime_error("WGPU adapter or device request failed");
		}

		WGPUAdapterProperties adapterProperties{};
		wgpuAdapterGetProperties(m_adapter, &adapterProperties);
		SPDLOG_INFO("Using hardware adapter {} ({})", adapterProperties.name, adapterProperties.deviceID);

		// Hook up error callbacks
		wgpuDeviceSetUncapturedErrorCallback(m_device, wgpuErrorCallback, nullptr);

		// Retrieve the associated device queue
		m_queue = wgpuDeviceGetQueue(m_device);
		if (!m_queue) {
			throw std::runtime_error("WGPU failed to get queue from render device");
		}

		// Configure the WebGPU render surface
		m_surfaceInfo = getSurfaceInfo(m_surface, m_adapter);

		WGPUSurfaceConfiguration surfaceConfig{};
		surfaceConfig.nextInChain = nullptr;
		surfaceConfig.device = m_device;
		surfaceConfig.format = m_surfaceInfo.preferredFormat;
		surfaceConfig.usage = WGPUTextureUsage_RenderAttachment;
		surfaceConfig.viewFormatCount = 0;
		surfaceConfig.viewFormats = nullptr;
		surfaceConfig.alphaMode = WGPUCompositeAlphaMode_Auto;
		surfaceConfig.width = m_framebufferSize.width;
		surfaceConfig.height = m_framebufferSize.height;
		surfaceConfig.presentMode = m_surfaceInfo.currentPresentMode;

		wgpuSurfaceConfigure(m_surface, &surfaceConfig);
		SPDLOG_INFO("Configured WebGPU surface:");
		SPDLOG_INFO("  Has SRGB support:      {}", m_surfaceInfo.isSRGB);
		SPDLOG_INFO("  Has mailbox present:   {}", m_surfaceInfo.hasMailboxPresent);
		SPDLOG_INFO("  Has immediate present: {}", m_surfaceInfo.hasImmediatePresent);

		// Done!
		SPDLOG_INFO("Initialized WebGPU render backend");
	}

	RenderBackend::~RenderBackend()
	{
		wgpuQueueRelease(m_queue);
		wgpuDeviceRelease(m_device);
		wgpuAdapterRelease(m_adapter);
		wgpuSurfaceRelease(m_surface);
		wgpuInstanceRelease(m_instance);
	}

	bool RenderBackend::newFrame(FrameState& state)
	{
		// Acquire next surface texture
		WGPUSurfaceTexture surfaceTexture{};
		wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);

		// Handle acquisition errors
		bool acquisitionError = false;
		switch (surfaceTexture.status)
		{
		case WGPUSurfaceGetCurrentTextureStatus_Success:
			break;
		case WGPUSurfaceGetCurrentTextureStatus_Timeout:
		case WGPUSurfaceGetCurrentTextureStatus_Outdated:
		case WGPUSurfaceGetCurrentTextureStatus_Lost:
			acquisitionError = true;
			break;
		case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
		case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
		default:
			throw std::runtime_error("WGPU surface texture cannot be acquired");
		}

		// Avoid rendering to suboptimal or unacquired textures.
		if (surfaceTexture.suboptimal || acquisitionError)
		{
			if (surfaceTexture.texture) { // Release surface texture if not null
				wgpuTextureRelease(surfaceTexture.texture);
			}

			return false;
		}

		// Populate frame state
		state.swapTexture = surfaceTexture.texture;
		state.swapTextureView = wgpuTextureCreateView(surfaceTexture.texture, nullptr /* default view */);

		return true;
	}

	void RenderBackend::present(FrameState& state)
	{
		wgpuSurfacePresent(m_surface);

		// Clean up the frame state :)
		wgpuTextureViewRelease(state.swapTextureView);
		wgpuTextureRelease(state.swapTexture);
	}

	void RenderBackend::submit(size_t commandCount, WGPUCommandBuffer const* pCommands)
	{
		wgpuQueueSubmit(m_queue, commandCount, pCommands);
	}

	void RenderBackend::resizeSwapBuffers(FramebufferSize const& size)
	{
		m_framebufferSize = size;

		WGPUSurfaceConfiguration surfaceConfig{};
		surfaceConfig.nextInChain = nullptr;
		surfaceConfig.device = m_device;
		surfaceConfig.format = m_surfaceInfo.preferredFormat;
		surfaceConfig.usage = WGPUTextureUsage_RenderAttachment;
		surfaceConfig.viewFormatCount = 0;
		surfaceConfig.viewFormats = nullptr;
		surfaceConfig.alphaMode = WGPUCompositeAlphaMode_Auto;
		surfaceConfig.width = m_framebufferSize.width;
		surfaceConfig.height = m_framebufferSize.height;
		surfaceConfig.presentMode = m_surfaceInfo.currentPresentMode;

		wgpuSurfaceConfigure(m_surface, &surfaceConfig);
	}

	WGPUAdapter RenderBackend::requestAdapter(WGPUInstance instance, WGPURequestAdapterOptions* pOptions) const
	{
		assert(pOptions != nullptr);
		struct UserData
		{
			bool		done = false;
			WGPUAdapter adapter = nullptr;
		} ud;

		auto const callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* pMessage, void* pUserData)
			{
				UserData& ud = *static_cast<UserData*>(pUserData);
				if (status == WGPURequestAdapterStatus_Success) {
					ud.adapter = adapter;
				}
				else {
					SPDLOG_ERROR("WGPU adapter request failed: {}", pMessage);
				}

				ud.done = true;
			};

		wgpuInstanceRequestAdapter(instance, pOptions, callback, &ud);
		return ud.adapter;
	}

	WGPUDevice RenderBackend::requestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor* pDesc) const
	{
		assert(pDesc != nullptr);
		struct UserData
		{
			bool		done = false;
			WGPUDevice	device = nullptr;
		} ud;

		auto const callback = [](WGPURequestDeviceStatus status, WGPUDevice device, char const* pMessage, void* pUserData)
			{
				UserData& ud = *static_cast<UserData*>(pUserData);
				if (status == WGPURequestDeviceStatus_Success) {
					ud.device = device;
				}
				else {
					SPDLOG_ERROR("WGPU device request failed: {}", pMessage);
				}

				ud.done = true;
			};

		wgpuAdapterRequestDevice(adapter, pDesc, callback, &ud);
		return ud.device;
	}

	RenderBackend::SurfaceInfo RenderBackend::getSurfaceInfo(WGPUSurface surface, WGPUAdapter adapter) const
	{
		SurfaceInfo info{};
		WGPUSurfaceCapabilities surfaceCaps{};
		wgpuSurfaceGetCapabilities(surface, adapter, &surfaceCaps);

		// Find surface format support
		for (size_t i = 0; i < surfaceCaps.formatCount; i++)
		{
			if (surfaceCaps.formats[i] == WGPUTextureFormat_RGBA8UnormSrgb
				|| surfaceCaps.formats[i] == WGPUTextureFormat_BGRA8UnormSrgb)
			{
				info.isSRGB = true;
				info.preferredFormat = surfaceCaps.formats[i];
				break;
			}

			if (surfaceCaps.formats[i] == WGPUTextureFormat_RGBA8Unorm
				|| surfaceCaps.formats[i] == WGPUTextureFormat_BGRA8Unorm)
			{
				info.isSRGB = false;
				info.preferredFormat = surfaceCaps.formats[i];
				break;
			}
		}

		// Find present mode support
		for (size_t i = 0; i < surfaceCaps.presentModeCount; i++)
		{
			if (surfaceCaps.presentModes[i] == WGPUPresentMode_Mailbox) {
				info.hasMailboxPresent = true;
			}

			if (surfaceCaps.presentModes[i] == WGPUPresentMode_Immediate) {
				info.hasImmediatePresent = true;
			}
		}

		// Set initial present mode to FIFO since it's always supported
		info.currentPresentMode = WGPUPresentMode_Fifo;

		return info;
	}
} // namespace gfx
