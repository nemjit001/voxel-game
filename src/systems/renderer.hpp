#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "rendering/draw_list.hpp"
#include "rendering/render_backend.hpp"

#define RENDERER_PASS_OPAQUE "Opaque Pass"

/// @brief Uniform camera data.
struct CameraUniform
{
    glm::mat4 view;
    glm::mat4 project;
    glm::mat4 viewproject;
};

/// @brief Uniform material data.
struct MaterialUniform
{
    glm::vec3   albedoColor; float _pad0;
    uint32_t    hasAlbedoMap;
    uint32_t    hasNormalMap;
};

/// @brief Uniform object transform data.
struct ObjectTranformUniform
{
    glm::mat4 modelTransform;
    glm::mat4 normalTransform;
};

/// @brief The Renderer system handles rendering the game world entities.
class Renderer
{
public:
    /// @brief Create a new renderer.
    /// @param renderbackend Render backend shared pointer.
    Renderer(std::shared_ptr<gfx::RenderBackend> renderbackend);
    ~Renderer();

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;

    /// @brief Render the next game frame.
    /// @param registry ECS registry to use for rendering.
    void render(entt::registry const& registry);

    /// @brief Handle a window resize event in the renderer.
    /// @param width 
    /// @param height 
    void onResize(uint32_t width, uint32_t height);

private:
    /// @brief Upload GPU scene data that has changed this frame.
    /// @param registry 
    void uploadSceneData(entt::registry const& registry);

    /// @brief Prepare the game frame state.
    /// @param registry 
    /// @retrurn A drawlist containing all render pass draw commands.
    gfx::DrawList prepare(entt::registry const& registry);

    /// @brief Execute the game frame render state.
    /// @param registry 
    void execute(gfx::FrameState frame, gfx::DrawList const& drawlist);

private:
    std::shared_ptr<gfx::RenderBackend> m_renderbackend;

    // Render pass resources
    WGPUTexture                 m_depthStencilTarget            = nullptr;
    WGPUTextureView             m_depthStencilTargetView        = nullptr;

    WGPUBuffer                  m_cameraDataUBO                 = nullptr;
    WGPUBuffer                  m_objectTransformDataUBO        = nullptr;
    WGPUBuffer                  m_materialDataUBO               = nullptr;

    // Pipeline resources
    WGPUBindGroupLayout         m_sceneDataBindGroupLayout      = nullptr;
    WGPUBindGroupLayout         m_objectDataBindGroupLayout     = nullptr;
    WGPUBindGroupLayout         m_materialDataBindGroupLayout   = nullptr;
    WGPUPipelineLayout          m_pipelineLayout                = nullptr;
    WGPURenderPipeline          m_pipeline                      = nullptr;

    // Pipeline bind groups
    WGPUBindGroup               m_sceneDataBindGroup            = nullptr;
    WGPUBindGroup               m_objectDataBindGroup           = nullptr;
    std::vector<WGPUBindGroup>  m_materialDataBindGroups        = {};
};
