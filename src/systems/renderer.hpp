#pragma once

#include <cstdint>
#include <memory>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "rendering/render_backend.hpp"

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
    void render(entt::registry& registry);

    /// @brief Handle a window resize event in the renderer.
    /// @param width 
    /// @param height 
    void onResize(uint32_t width, uint32_t height);

private:
    std::shared_ptr<gfx::RenderBackend> m_renderbackend;

    // Render pass resources
    WGPUTexture         m_depthStencilTarget            = nullptr;
    WGPUTextureView     m_depthStencilTargetView        = nullptr;

    // Pipeline resources
    WGPUBindGroupLayout m_sceneDataBindGroupLayout      = nullptr;
    WGPUBindGroupLayout m_materialDataBindGroupLayout   = nullptr;
    WGPUBindGroupLayout m_objectDataBindGroupLayout     = nullptr;
    WGPUPipelineLayout  m_pipelineLayout                = nullptr;
    WGPURenderPipeline  m_pipeline                      = nullptr;
};
