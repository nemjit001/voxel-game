#include "game.hpp"

#include <cassert>
#include <spdlog/spdlog.h>

#include "macros.hpp"
#include "core/files.hpp"
#include "rendering/vertex.hpp"

static constexpr char const*    WINDOW_TITLE            = "Voxel Game";
static constexpr uint32_t       DEFAULT_WINDOW_WIDTH    = 1280;
static constexpr uint32_t       DEFAULT_WINDOW_HEIGHT   = 720;

static void windowResizeCallback(GLFWwindow* pWindow, int width, int height)
{
    Game* pThat = reinterpret_cast<Game*>(glfwGetWindowUserPointer(pWindow));
    assert(pThat != nullptr);

    pThat->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

static void keyCallback(GLFWwindow* pWindow, int keycode, int scancode, int action, int mods)
{
    Game* pThat = reinterpret_cast<Game*>(glfwGetWindowUserPointer(pWindow));
    assert(pThat != nullptr);
}

static void mousePosCallback(GLFWwindow* pWindow, double xpos, double ypos)
{
    Game* pThat = reinterpret_cast<Game*>(glfwGetWindowUserPointer(pWindow));
    assert(pThat != nullptr);
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
    glfwSetWindowSizeCallback(m_pWindow, windowResizeCallback);
    glfwSetKeyCallback(m_pWindow, keyCallback);
    glfwSetCursorPosCallback(m_pWindow, mousePosCallback);

    // Initialize render backend
    SPDLOG_INFO("Initializing render backend");
    m_renderbackend = std::make_shared<RenderBackend>(m_pWindow);

    // Set up a graphics pipeline for rendering
    {
        // Create pipeline layout
        WGPUPipelineLayoutDescriptor layoutDesc{};
        layoutDesc.nextInChain = nullptr;
        layoutDesc.label = "Pipeline Layout";
        layoutDesc.bindGroupLayoutCount = 0;
        layoutDesc.bindGroupLayouts = nullptr;

        m_pipelineLayout = wgpuDeviceCreatePipelineLayout(m_renderbackend->getDevice(), &layoutDesc);

        // Load shader module
        std::vector<uint8_t> const shaderBinary = fs::readBinaryFile(fs::getFullAssetPath("assets/shaders/shaders.wgsl"));
        std::string const shaderCode(shaderBinary.begin(), shaderBinary.end()); // Convert byte vector to string

        WGPUShaderModuleWGSLDescriptor wgslShaderDesc{};
        wgslShaderDesc.chain.next = nullptr;
        wgslShaderDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
        wgslShaderDesc.code = shaderCode.data();

        WGPUShaderModuleDescriptor shaderDesc{};
        shaderDesc.nextInChain = &wgslShaderDesc.chain;
        shaderDesc.label = "Shader";
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;

        WGPUShaderModule shader = wgpuDeviceCreateShaderModule(m_renderbackend->getDevice(), &shaderDesc);

        // Set up pipeline state
        WGPUVertexAttribute vertexAttributes[] = {
            { WGPUVertexFormat_Float32x3, offsetof(Vertex, position), 0 },
            { WGPUVertexFormat_Float32x3, offsetof(Vertex, normal), 1 },
            { WGPUVertexFormat_Float32x3, offsetof(Vertex, tangent), 2 },
            { WGPUVertexFormat_Float32x2, offsetof(Vertex, texcoord), 3 },
        };

        WGPUVertexBufferLayout vertexBufferLayouts[] = {
            { sizeof(Vertex), WGPUVertexStepMode_Vertex, std::size(vertexAttributes), vertexAttributes},
        };

        WGPUVertexState vertexState{};
        vertexState.nextInChain = nullptr;
        vertexState.module = shader;
        vertexState.entryPoint = "VSStaticVert";
        vertexState.constantCount = 0;
        vertexState.constants = nullptr;
        vertexState.bufferCount = std::size(vertexBufferLayouts);
        vertexState.buffers = vertexBufferLayouts;

        WGPUBlendState colorBlendState{};
        colorBlendState.color.srcFactor = WGPUBlendFactor_One;
        colorBlendState.color.dstFactor = WGPUBlendFactor_Zero;
        colorBlendState.color.operation = WGPUBlendOperation_Add;
        colorBlendState.alpha.srcFactor = WGPUBlendFactor_One;
        colorBlendState.alpha.dstFactor = WGPUBlendFactor_Zero;
        colorBlendState.alpha.operation = WGPUBlendOperation_Add;

        WGPUColorTargetState colorTarget{};
        colorTarget.nextInChain = nullptr;
        colorTarget.format = m_renderbackend->getSwapFormat();
        colorTarget.blend = &colorBlendState;
        colorTarget.writeMask = WGPUColorWriteMask_All;

        WGPUFragmentState fragmentState{};
        fragmentState.nextInChain = nullptr;
        fragmentState.module = shader;
        fragmentState.entryPoint = "FSForwardShading";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;

        WGPUPrimitiveState primitiveState{};
        primitiveState.nextInChain = nullptr;
        primitiveState.topology = WGPUPrimitiveTopology_TriangleList;
        primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
        primitiveState.frontFace = WGPUFrontFace_CCW;
        primitiveState.cullMode = WGPUCullMode_None;

        WGPUMultisampleState multisampleState{};
        multisampleState.nextInChain = nullptr;
        multisampleState.count = 1;
        multisampleState.mask = UINT32_MAX;
        multisampleState.alphaToCoverageEnabled = false;

        WGPURenderPipelineDescriptor pipelineDesc{};
        pipelineDesc.nextInChain = nullptr;
        pipelineDesc.label = "Render Pipeline";
        pipelineDesc.layout = m_pipelineLayout;
        pipelineDesc.vertex = vertexState;
        pipelineDesc.fragment = &fragmentState;
        pipelineDesc.primitive = primitiveState;
        pipelineDesc.depthStencil = nullptr;
        pipelineDesc.multisample = multisampleState;

        m_pipeline = wgpuDeviceCreateRenderPipeline(m_renderbackend->getDevice(), &pipelineDesc);
        wgpuShaderModuleRelease(shader);
    }

    // We are initialized!
    SPDLOG_INFO("Initialized!");
    m_running = true;
    m_frameTimer.reset(); //< 1st frame does not need to know how long it took to start up the game...
}

Game::~Game()
{
    SPDLOG_INFO("Shutting down game...");

    // TEMP destroy rendering stuff
    wgpuRenderPipelineRelease(m_pipeline);
    wgpuPipelineLayoutRelease(m_pipelineLayout);

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
        wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);

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
