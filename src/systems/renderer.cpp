#include "renderer.hpp"

#include <set>
#include <unordered_map>
#include <vector>
#include <spdlog/spdlog.h>

#include "core/files.hpp"
#include "rendering/vertex_layout.hpp"
#include "components/camera.hpp"
#include "components/render_component.hpp"
#include "components/transform.hpp"

Renderer::Renderer(std::shared_ptr<gfx::RenderBackend> renderbackend)
	:
	m_renderbackend(renderbackend)
{
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
        sceneDataCameraBinding.buffer.minBindingSize = 0;

        WGPUBindGroupLayoutEntry sceneDataBindGroupEntries[] = { sceneDataCameraBinding, };
        WGPUBindGroupLayoutDescriptor sceneDataBindGroupLayoutDesc{};
        sceneDataBindGroupLayoutDesc.nextInChain = nullptr;
        sceneDataBindGroupLayoutDesc.label = "Scene Data Bind Group Layout";
        sceneDataBindGroupLayoutDesc.entryCount = std::size(sceneDataBindGroupEntries);
        sceneDataBindGroupLayoutDesc.entries = sceneDataBindGroupEntries;

        m_sceneDataBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_renderbackend->getDevice(), &sceneDataBindGroupLayoutDesc);

        // Create material data bind group
        WGPUBindGroupLayoutEntry materialDataMaterialBinding{};
        materialDataMaterialBinding.nextInChain = nullptr;
        materialDataMaterialBinding.binding = 0;
        materialDataMaterialBinding.visibility = WGPUShaderStage_Fragment;
        materialDataMaterialBinding.buffer.nextInChain = nullptr;
        materialDataMaterialBinding.buffer.type = WGPUBufferBindingType_Uniform;
        materialDataMaterialBinding.buffer.hasDynamicOffset = false;
        materialDataMaterialBinding.buffer.minBindingSize = 0;

        WGPUBindGroupLayoutEntry materialDataLinearSamplerBinding{};
        materialDataLinearSamplerBinding.nextInChain = nullptr;
        materialDataLinearSamplerBinding.binding = 1;
        materialDataLinearSamplerBinding.visibility = WGPUShaderStage_Fragment;
        materialDataLinearSamplerBinding.sampler.nextInChain = nullptr;
        materialDataLinearSamplerBinding.sampler.type = WGPUSamplerBindingType_Filtering;

        WGPUBindGroupLayoutEntry materialDataAlbedoMapBinding{};
        materialDataAlbedoMapBinding.nextInChain = nullptr;
        materialDataAlbedoMapBinding.binding = 2;
        materialDataAlbedoMapBinding.visibility = WGPUShaderStage_Fragment;
        materialDataAlbedoMapBinding.texture.nextInChain = nullptr;
        materialDataAlbedoMapBinding.texture.sampleType = WGPUTextureSampleType_Float;
        materialDataAlbedoMapBinding.texture.viewDimension = WGPUTextureViewDimension_2D;
        materialDataAlbedoMapBinding.texture.multisampled = false;

        WGPUBindGroupLayoutEntry materialDataNormalMapBinding{};
        materialDataNormalMapBinding.nextInChain = nullptr;
        materialDataNormalMapBinding.binding = 3;
        materialDataNormalMapBinding.visibility = WGPUShaderStage_Fragment;
        materialDataNormalMapBinding.texture.nextInChain = nullptr;
        materialDataNormalMapBinding.texture.sampleType = WGPUTextureSampleType_Float;
        materialDataNormalMapBinding.texture.viewDimension = WGPUTextureViewDimension_2D;
        materialDataNormalMapBinding.texture.multisampled = false;

        WGPUBindGroupLayoutEntry materialDataBindGroupEntries[] = { materialDataMaterialBinding, materialDataLinearSamplerBinding, materialDataAlbedoMapBinding, materialDataNormalMapBinding };
        WGPUBindGroupLayoutDescriptor materialDataBindGroupLayoutDesc{};
        materialDataBindGroupLayoutDesc.nextInChain = nullptr;
        materialDataBindGroupLayoutDesc.label = "Material Data Bind Group Layout";
        materialDataBindGroupLayoutDesc.entryCount = std::size(materialDataBindGroupEntries);
        materialDataBindGroupLayoutDesc.entries = materialDataBindGroupEntries;

        m_materialDataBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_renderbackend->getDevice(), &materialDataBindGroupLayoutDesc);

        // Create object data bind group
        WGPUBindGroupLayoutEntry objectDataObjectTransformBinding{};
        objectDataObjectTransformBinding.nextInChain = nullptr;
        objectDataObjectTransformBinding.binding = 0;
        objectDataObjectTransformBinding.visibility = WGPUShaderStage_Vertex;
        objectDataObjectTransformBinding.buffer.nextInChain = nullptr;
        objectDataObjectTransformBinding.buffer.type = WGPUBufferBindingType_Uniform;
        objectDataObjectTransformBinding.buffer.hasDynamicOffset = false;
        objectDataObjectTransformBinding.buffer.minBindingSize = 0;

        WGPUBindGroupLayoutEntry objectDataBindGroupEntries[] = { objectDataObjectTransformBinding, };
        WGPUBindGroupLayoutDescriptor objectDataBindGroupLayoutDesc{};
        objectDataBindGroupLayoutDesc.nextInChain = nullptr;
        objectDataBindGroupLayoutDesc.label = "Object Data Bind Group Layout";
        objectDataBindGroupLayoutDesc.entryCount = std::size(objectDataBindGroupEntries);
        objectDataBindGroupLayoutDesc.entries = objectDataBindGroupEntries;

        m_objectDataBindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_renderbackend->getDevice(), &objectDataBindGroupLayoutDesc);

        // Create pipeline layout
        WGPUBindGroupLayout bindGroupLayouts[] = { m_sceneDataBindGroupLayout, m_materialDataBindGroupLayout, m_objectDataBindGroupLayout, };
        WGPUPipelineLayoutDescriptor layoutDesc{};
        layoutDesc.nextInChain = nullptr;
        layoutDesc.label = "Pipeline Layout";
        layoutDesc.bindGroupLayoutCount = std::size(bindGroupLayouts);
        layoutDesc.bindGroupLayouts = bindGroupLayouts;

        m_pipelineLayout = wgpuDeviceCreatePipelineLayout(m_renderbackend->getDevice(), &layoutDesc);

        // Load shader module
        std::string const shaderFilePath = core::fs::getFullAssetPath("assets/shaders/shaders.wgsl");
        std::vector<uint8_t> const shaderBinary = core::fs::readBinaryFile(shaderFilePath);
        std::string const shaderCode(shaderBinary.begin(), shaderBinary.end()); // Convert byte vector to string
        if (shaderCode.empty()) {
            SPDLOG_ERROR("Failed to load shader file from path {}", shaderFilePath); // FIXME(nemtji001): Throw fatal error here
        }

        WGPUShaderModuleWGSLDescriptor wgslShaderDesc{};
        wgslShaderDesc.chain.next = nullptr;
        wgslShaderDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
        wgslShaderDesc.code = shaderCode.data();

        WGPUShaderModuleDescriptor shaderDesc{};
        shaderDesc.nextInChain = &wgslShaderDesc.chain;
        shaderDesc.label = "Shader";
#if     WEBGPU_BACKEND_WGPU
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;
#endif  // WEBGPU_BACKEND_WGPU

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
}

Renderer::~Renderer()
{
    // Destroy pipeline state
    wgpuRenderPipelineRelease(m_pipeline);
    wgpuPipelineLayoutRelease(m_pipelineLayout);
    wgpuBindGroupLayoutRelease(m_objectDataBindGroupLayout);
    wgpuBindGroupLayoutRelease(m_materialDataBindGroupLayout);
    wgpuBindGroupLayoutRelease(m_sceneDataBindGroupLayout);

    // Destroy depth-stencil target
    wgpuTextureViewRelease(m_depthStencilTargetView);
    wgpuTextureRelease(m_depthStencilTarget);
}

void Renderer::render(entt::registry const& registry)
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

    // Prepare frame state
    prepare(registry);

    // Execute frame commands
    execute(frame);
}

void Renderer::onResize(uint32_t width, uint32_t height)
{
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

void Renderer::uploadSceneData(entt::registry const& registry)
{
    auto const objects = registry.view<RenderComponent, Transform>();

    // Gather updated host data
    // NOTE(nemjit001): Sets handle deduplication automatically, unordered sets are faster than std::set
    std::unordered_set<std::shared_ptr<gfx::Mesh>> dirtyMeshes{};
    std::unordered_set<std::shared_ptr<gfx::Texture>> dirtyTextures{};
    {
        for (auto const& [_entity, object, _transform] : objects.each())
        {
            // Skip null data
            if (!object.mesh || !object.material)
            {
                SPDLOG_WARN("Entity {} has null components in render data", static_cast<size_t>(_entity));
                continue;
            }

            // Track dirty meshes
            if (object.mesh && object.mesh->isDirty()) {
                dirtyMeshes.emplace(object.mesh);
            }

            // Track dirty textures
            if (object.material->albedoTexture && object.material->albedoTexture->isDirty()) {
                dirtyTextures.emplace(object.material->albedoTexture);
            }

            if (object.material->normalTexture && object.material->normalTexture->isDirty()) {
                dirtyTextures.emplace(object.material->normalTexture);
            }
        }
    }

    // Create and populate GPU objects with host-side data
    {
        for (auto& mesh : dirtyMeshes)
        {
            // Create buffers
            WGPUBufferDescriptor vertexBufferDesc{};
            vertexBufferDesc.nextInChain = nullptr;
            vertexBufferDesc.label = "Vertex Buffer (managed)";
            vertexBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
            vertexBufferDesc.size = mesh->vertexCount() * sizeof(gfx::Vertex);
            vertexBufferDesc.mappedAtCreation = false;

            WGPUBufferDescriptor indexBufferDesc{};
            indexBufferDesc.nextInChain = nullptr;
            indexBufferDesc.label = "Index Buffer (managed)";
            indexBufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
            indexBufferDesc.size = mesh->indexCount() * sizeof(gfx::IndexType);
            indexBufferDesc.mappedAtCreation = false;

            WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(m_renderbackend->getDevice(), &vertexBufferDesc);
            WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(m_renderbackend->getDevice(), &indexBufferDesc);
            SPDLOG_TRACE("Created mesh buffers (vertex bytes: {} | index bytes: {})", vertexBufferDesc.size, indexBufferDesc.size);

            // Upload buffer data
            std::vector<gfx::Vertex> vertices{};
            std::vector<gfx::IndexType> indices{};
            mesh->getBuffers(vertices, indices);
            wgpuQueueWriteBuffer(m_renderbackend->getQueue(), vertexBuffer, 0, vertices.data(), wgpuBufferGetSize(vertexBuffer));
            wgpuQueueWriteBuffer(m_renderbackend->getQueue(), indexBuffer, 0, indices.data(), wgpuBufferGetSize(indexBuffer));

            // Update mesh
            mesh->setVertexBuffer(vertexBuffer);
            mesh->setIndexBuffer(indexBuffer);
            mesh->clearDirtyFlag(); // done :)
        }

        for (auto& texture : dirtyTextures)
        {
            // Get texture data
            gfx::TextureDimensions const dimensions = texture->dimensions();
            gfx::TextureExtent const extent = texture->extent();
            uint8_t const components = texture->components();

            // Parse dimensions
            WGPUTextureDimension dim = WGPUTextureDimension_Force32;
            switch (dimensions)
            {
            case gfx::TextureDimensions::Dim1D:
                dim = WGPUTextureDimension_1D;
                break;
            case gfx::TextureDimensions::Dim2D:
                dim = WGPUTextureDimension_2D;
                break;
            case gfx::TextureDimensions::Dim3D:
                dim = WGPUTextureDimension_3D;
                break;
            default:
                break;
            }

            // Guess a good format based on components
            WGPUTextureFormat format = WGPUTextureFormat_Undefined;
            switch (components)
            {
            case 1:
                format = WGPUTextureFormat_R8Unorm;
                break;
            case 2:
                format = WGPUTextureFormat_RG8Unorm;
                break;
            case 3:
                format = WGPUTextureFormat_RGBA8Unorm; // Uses RGBA since RGB is not available in WebGPU
                break;
            case 4:
                format = WGPUTextureFormat_RGBA8Unorm;
                break;
            default:
                break;
            }

            // Create texture
            WGPUTextureDescriptor textureDesc{};
            textureDesc.nextInChain = nullptr;
            textureDesc.label = "Image Texture (managed)";
            textureDesc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
            textureDesc.dimension = dim;
            textureDesc.size.width = extent.width;
            textureDesc.size.height = extent.height;
            textureDesc.size.depthOrArrayLayers = extent.depthOrArrayLayers;
            textureDesc.format = format;
            textureDesc.mipLevelCount = 1; // TODO(nemjit001): implement mip levels on host side, then upload as bytes into texture
            textureDesc.sampleCount = 1;
            textureDesc.viewFormatCount = 0;
            textureDesc.viewFormats = nullptr;

            WGPUTexture gpuTexture = wgpuDeviceCreateTexture(m_renderbackend->getDevice(), &textureDesc);
            SPDLOG_TRACE("Created texture handle (size: {}x{}x{})", textureDesc.size.width, textureDesc.size.height, textureDesc.size.depthOrArrayLayers);

            // Upload texture data
            WGPUImageCopyTexture destination{};
            destination.nextInChain = nullptr;
            destination.texture = gpuTexture;
            destination.mipLevel = 0;
            destination.origin = { 0, 0, 0 }; // Always 0, 0, 0 for now
            destination.aspect = WGPUTextureAspect_All;

            WGPUTextureDataLayout layout{};
            layout.nextInChain = nullptr;
            layout.offset = 0;
            layout.bytesPerRow = components * extent.width;
            layout.rowsPerImage = extent.height;

            size_t const dataSize = extent.width * extent.height * extent.depthOrArrayLayers * components;
            wgpuQueueWriteTexture(m_renderbackend->getQueue(), &destination, texture->data(), dataSize, &layout, &textureDesc.size);

            // Update texture
            texture->setTexture(gpuTexture);
            texture->clearDirtyFlag(); // Done :)
        }
    }
}

void Renderer::prepare(entt::registry const& registry)
{
    // Gather render data from ECS registry
    auto const cameras = registry.view<Camera, Transform>();
    auto const objects = registry.view<RenderComponent, Transform>();

    // TODO(nemjit001): Gather uniforms & upload
}

void Renderer::execute(gfx::FrameState frame)
{
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
#if     !WEBGPU_BACKEND_WGPU
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif  // !WEBGPU_BACKEND_WGPU

    WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
    depthStencilAttachment.view = m_depthStencilTargetView;
    depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp = WGPUStoreOp_Discard;
    depthStencilAttachment.depthClearValue = 1.0F;
    depthStencilAttachment.depthReadOnly = false;
    depthStencilAttachment.stencilLoadOp = WGPULoadOp_Clear;
    depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Discard;
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
    gfx::FramebufferSize const swapFramebufferSize = m_renderbackend->getFramebufferSize();
    wgpuRenderPassEncoderSetViewport(renderPass, 0.0F, 0.0F, static_cast<float>(swapFramebufferSize.width), static_cast<float>(swapFramebufferSize.height), 0.0F, 1.0F);
    wgpuRenderPassEncoderSetScissorRect(renderPass, 0, 0, swapFramebufferSize.width, swapFramebufferSize.height);
    wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);

    // Finish opaque object pass
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
