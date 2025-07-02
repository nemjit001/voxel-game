#include "game.hpp"

#include <cassert>
#include <spdlog/spdlog.h>

#include "macros.hpp"
#include "core/files.hpp"
#include "rendering/vertex_layout.hpp"

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
    SPDLOG_INFO("Program directory: {}", core::fs::getProgramDirectory());
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
    m_renderbackend = std::make_shared<gfx::RenderBackend>(m_pWindow);

    // Set up a depth-stencil target for rendering
    {
        gfx::FramebufferSize const swapFramebufferSize = m_renderbackend->getFramebufferSize();
        WGPUTextureDescriptor depthStencilTargetDesc{};
        depthStencilTargetDesc.nextInChain = nullptr;
        depthStencilTargetDesc.label = "Depth Stencil Target";
        depthStencilTargetDesc.usage = WGPUTextureUsage_RenderAttachment;
        depthStencilTargetDesc.dimension = WGPUTextureDimension_2D;
        depthStencilTargetDesc.size.width = swapFramebufferSize.width;
        depthStencilTargetDesc.size.height = swapFramebufferSize.height;
        depthStencilTargetDesc.size.depthOrArrayLayers = 1;
        depthStencilTargetDesc.format = WGPUTextureFormat_Depth24PlusStencil8;
        depthStencilTargetDesc.mipLevelCount = 1;
        depthStencilTargetDesc.sampleCount = 1;
        depthStencilTargetDesc.viewFormatCount = 0;
        depthStencilTargetDesc.viewFormats = nullptr;

        m_depthStencilTarget = wgpuDeviceCreateTexture(m_renderbackend->getDevice(), &depthStencilTargetDesc);
        m_depthStencilTargetView = wgpuTextureCreateView(m_depthStencilTarget, nullptr /* default view */);
    }

    // Set up a graphics pipeline for rendering
    {
        // Create scene data bind group
        WGPUBindGroupLayoutEntry sceneDataCameraBinding{};
        sceneDataCameraBinding.nextInChain = nullptr;
        sceneDataCameraBinding.binding = 0;
        sceneDataCameraBinding.visibility = WGPUShaderStage_Vertex;
        sceneDataCameraBinding.buffer.nextInChain = nullptr;
        sceneDataCameraBinding.buffer.type = WGPUBufferBindingType_Uniform;
        sceneDataCameraBinding.buffer.hasDynamicOffset = false;
        sceneDataCameraBinding.buffer.minBindingSize = 0; // Can be kept zero, should be sizeof(CameraUniform)

        WGPUBindGroupLayoutEntry sceneDataBindGroupEntries[] = { sceneDataCameraBinding, };
        WGPUBindGroupLayoutDescriptor sceneDataBindGroupLayoutDesc{};
        sceneDataBindGroupLayoutDesc.nextInChain = nullptr;
        sceneDataBindGroupLayoutDesc.label = "Scene Data Bind Group Layout";
        sceneDataBindGroupLayoutDesc.entryCount = std::size(sceneDataBindGroupEntries);
        sceneDataBindGroupLayoutDesc.entries = sceneDataBindGroupEntries;

        m_sceneDataBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_renderbackend->getDevice(), &sceneDataBindGroupLayoutDesc);

        // Create object data bind group
        WGPUBindGroupLayoutEntry objectDataObjectTransformBinding{};
        objectDataObjectTransformBinding.nextInChain = nullptr;
        objectDataObjectTransformBinding.binding = 0;
        objectDataObjectTransformBinding.visibility = WGPUShaderStage_Vertex;
        objectDataObjectTransformBinding.buffer.nextInChain = nullptr;
        objectDataObjectTransformBinding.buffer.type = WGPUBufferBindingType_Uniform;
        objectDataObjectTransformBinding.buffer.hasDynamicOffset = false;
        objectDataObjectTransformBinding.buffer.minBindingSize = 0; // Can be kept zero, should be sizeof(ObjectTransformUniform)

        WGPUBindGroupLayoutEntry objectDataBindGroupEntries[] = { objectDataObjectTransformBinding, };
        WGPUBindGroupLayoutDescriptor objectDataBindGroupLayoutDesc{};
        objectDataBindGroupLayoutDesc.nextInChain = nullptr;
        objectDataBindGroupLayoutDesc.label = "Object Data Bind Group Layout";
        objectDataBindGroupLayoutDesc.entryCount = std::size(objectDataBindGroupEntries);
        objectDataBindGroupLayoutDesc.entries = objectDataBindGroupEntries;

        m_objectDataBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_renderbackend->getDevice(), &objectDataBindGroupLayoutDesc);

        // Create pipeline layout
        WGPUBindGroupLayout bindGroupLayouts[] = { m_sceneDataBindGroupLayout, m_objectDataBindGroupLayout, };
        WGPUPipelineLayoutDescriptor layoutDesc{};
        layoutDesc.nextInChain = nullptr;
        layoutDesc.label = "Pipeline Layout";
        layoutDesc.bindGroupLayoutCount = std::size(bindGroupLayouts);
        layoutDesc.bindGroupLayouts = bindGroupLayouts;

        m_pipelineLayout = wgpuDeviceCreatePipelineLayout(m_renderbackend->getDevice(), &layoutDesc);

        // Load shader module
        std::vector<uint8_t> const shaderBinary = core::fs::readBinaryFile(core::fs::getFullAssetPath("assets/shaders/shaders.wgsl"));
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
            { WGPUVertexFormat_Float32x3, offsetof(gfx::Vertex, position), 0 },
            { WGPUVertexFormat_Float32x3, offsetof(gfx::Vertex, normal), 1 },
            { WGPUVertexFormat_Float32x3, offsetof(gfx::Vertex, tangent), 2 },
            { WGPUVertexFormat_Float32x2, offsetof(gfx::Vertex, texcoord), 3 },
        };

        WGPUVertexBufferLayout vertexBufferLayouts[] = {
            { sizeof(gfx::Vertex), WGPUVertexStepMode_Vertex, std::size(vertexAttributes), vertexAttributes},
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

        WGPUDepthStencilState depthStencilState{};
        depthStencilState.nextInChain = nullptr;
        depthStencilState.format = WGPUTextureFormat_Depth24PlusStencil8;
        depthStencilState.depthWriteEnabled = true;
        depthStencilState.depthCompare = WGPUCompareFunction_Less;
        depthStencilState.stencilFront = { WGPUCompareFunction_Always, WGPUStencilOperation_Keep, WGPUStencilOperation_Keep, WGPUStencilOperation_Keep };
        depthStencilState.stencilBack = { WGPUCompareFunction_Always, WGPUStencilOperation_Keep, WGPUStencilOperation_Keep, WGPUStencilOperation_Keep };
        depthStencilState.stencilReadMask = UINT32_MAX;
        depthStencilState.stencilWriteMask = UINT32_MAX;
        depthStencilState.depthBias = 0;
        depthStencilState.depthBiasSlopeScale = 0.0F;
        depthStencilState.depthBiasClamp = 0.0F;

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
        pipelineDesc.depthStencil = &depthStencilState;
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
    {
        // Destroy pipeline state
        wgpuRenderPipelineRelease(m_pipeline);
        wgpuPipelineLayoutRelease(m_pipelineLayout);
        wgpuBindGroupLayoutRelease(m_objectDataBindGroupLayout);
        wgpuBindGroupLayoutRelease(m_sceneDataBindGroupLayout);

        // Destroy depth-stencil target
        wgpuTextureViewRelease(m_depthStencilTargetView);
        wgpuTextureRelease(m_depthStencilTarget);
    }

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
        // Acquire new frame
        gfx::FrameState frame{};
        if (!m_renderbackend->newFrame(frame))
        {
            // Reconfigure framebuffer to be sure
            gfx::FramebufferSize const fbsize = m_renderbackend->getFramebufferSize();
            m_renderbackend->resizeSwapBuffers(fbsize);
            return;
        }

        // Prepare frame uniforms for render passes
        //

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

        WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
        depthStencilAttachment.view = m_depthStencilTargetView;
        depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
        depthStencilAttachment.depthStoreOp = WGPUStoreOp_Discard;
        depthStencilAttachment.depthClearValue = 1.0F;
        depthStencilAttachment.depthReadOnly = false;
        depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
        depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
        depthStencilAttachment.stencilClearValue = 0;
        depthStencilAttachment.stencilReadOnly = false;

        WGPURenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
        renderPassDesc.label = "Render Pass";
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        renderPassDesc.occlusionQuerySet = nullptr;
        renderPassDesc.timestampWrites = nullptr;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(frameCommandEncoder, &renderPassDesc);

        // Record opaque object pass
        wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);

        // Finish forward render pass
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

    // Recreate depth-stencil target
    {
        wgpuTextureViewRelease(m_depthStencilTargetView);
        wgpuTextureRelease(m_depthStencilTarget);

        gfx::FramebufferSize const swapFramebufferSize = m_renderbackend->getFramebufferSize();
        WGPUTextureDescriptor depthStencilTargetDesc{};
        depthStencilTargetDesc.nextInChain = nullptr;
        depthStencilTargetDesc.label = "Depth Stencil Target";
        depthStencilTargetDesc.usage = WGPUTextureUsage_RenderAttachment;
        depthStencilTargetDesc.dimension = WGPUTextureDimension_2D;
        depthStencilTargetDesc.size.width = swapFramebufferSize.width;
        depthStencilTargetDesc.size.height = swapFramebufferSize.height;
        depthStencilTargetDesc.size.depthOrArrayLayers = 1;
        depthStencilTargetDesc.format = WGPUTextureFormat_Depth24PlusStencil8;
        depthStencilTargetDesc.mipLevelCount = 1;
        depthStencilTargetDesc.sampleCount = 1;
        depthStencilTargetDesc.viewFormatCount = 0;
        depthStencilTargetDesc.viewFormats = nullptr;

        m_depthStencilTarget = wgpuDeviceCreateTexture(m_renderbackend->getDevice(), &depthStencilTargetDesc);
        m_depthStencilTargetView = wgpuTextureCreateView(m_depthStencilTarget, nullptr /* default view */);
    }
}

bool Game::isRunning() const
{
    return m_running && !glfwWindowShouldClose(m_pWindow);
}
