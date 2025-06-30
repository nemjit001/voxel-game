#include "game.hpp"

#include <cassert>
#include <spdlog/spdlog.h>

#include "macros.hpp"
#include "core/files.hpp"

static constexpr char const*    WINDOW_TITLE            = "Voxel Game";
static constexpr uint32_t       DEFAULT_WINDOW_WIDTH    = 1280;
static constexpr uint32_t       DEFAULT_WINDOW_HEIGHT   = 720;

static void windowSizeCallback(GLFWwindow* pWindow, int width, int height)
{
    Game* pThat = reinterpret_cast<Game*>(glfwGetWindowUserPointer(pWindow));
    assert(pThat != nullptr);

    pThat->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

Game::Game()
{
    // Dump some basic info
    SPDLOG_INFO("Initial window size: {} x {}", DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    SPDLOG_INFO("Program directory: {}", fs::getProgramDirectory());
#if     GAME_BUILD_TYPE_DEBUG
    SPDLOG_WARN("Running Debug build");
#endif  // GAME_BUILD_TYPE_DEBUG

    // Intialize platform layer
    SPDLOG_INFO("Initializing platform layer");
    if (glfwInit() != GLFW_TRUE) {
        return;
    }

    // Initialize game window
    SPDLOG_INFO("Initializing game window");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_pWindow = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!m_pWindow) {
        return;
    }

    // Hook up window callbacks
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetWindowSizeCallback(m_pWindow, windowSizeCallback);

    // Initialize render backend
    SPDLOG_INFO("Initializing render backend");
    m_renderbackend = std::make_shared<RenderBackend>(m_pWindow);

    // Initialize game world
    SPDLOG_INFO("Initializing game world");
    m_world = std::make_shared<World>();

    // We are initialized!
    SPDLOG_INFO("Initialized!");
    m_running = true;
    m_frameTimer.reset(); //< 1st frame does not need to know how long it took to start up the game...
}

Game::~Game()
{
    SPDLOG_INFO("Shutting down game...");

    // Destroy platform window
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

void Game::update()
{
    // Start frame & tick frame timer
    m_frameTimer.tick();
    SPDLOG_TRACE("Frame time: {8.2f} ms", m_frameTimer.delta());

    // Handle platform events
    glfwPollEvents();

    // Handle system updates
    //

    // Render frame
    {
        FrameState frame{};
        if (!m_renderbackend->newFrame(frame))
        {
            // Reconfigure framebuffer to be sure
            FramebufferSize const fbsize = m_renderbackend->getFramebufferSize();
            m_renderbackend->resizeSwapBuffers(fbsize);
            return;
        }

        // Start command recording for frame
        WGPUCommandEncoderDescriptor encoderDesc{};
        encoderDesc.nextInChain = nullptr;
        encoderDesc.label = "Frame Command Encoder";
        WGPUCommandEncoder frameCommandEncoder = wgpuDeviceCreateCommandEncoder(m_renderbackend->getDevice(), &encoderDesc);

        // Start render pass
        WGPURenderPassColorAttachment colorAttachment{};
        colorAttachment.nextInChain = nullptr;
        colorAttachment.view = frame.swapTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = WGPUColor{ 0.0F, 0.0F, 0.0F, 0.0F };

        WGPURenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
        renderPassDesc.label = "Render Pass";
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.occlusionQuerySet = nullptr;
        renderPassDesc.timestampWrites = nullptr;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(frameCommandEncoder, &renderPassDesc);
        wgpuRenderPassEncoderEnd(renderPass);

        // Finish command recording
        WGPUCommandBufferDescriptor commandBufDesc{};
        commandBufDesc.nextInChain = nullptr;
        commandBufDesc.label = "Frame Commands";
        WGPUCommandBuffer frameCommands = wgpuCommandEncoderFinish(frameCommandEncoder, &commandBufDesc);

        // Submit work & present
        m_renderbackend->submit(1, &frameCommands);
        m_renderbackend->present(frame);

        wgpuCommandBufferRelease(frameCommands);
        wgpuRenderPassEncoderRelease(renderPass);
        wgpuCommandEncoderRelease(frameCommandEncoder);
    }
}

void Game::onResize(uint32_t width, uint32_t height)
{
    SPDLOG_INFO("Window resized ({} x {})", width, height);

    m_renderbackend->resizeSwapBuffers({ width, height });
}

bool Game::isRunning() const
{
    return m_running && !glfwWindowShouldClose(m_pWindow);
}
