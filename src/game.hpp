#pragma once

#include <memory>
#include <GLFW/glfw3.h>

#include "core/timer.hpp"
#include "rendering/render_backend.hpp"

/// @brief The Game class binds all different game systems together into a cohesive whole.
class Game
{
public:
    Game();
    ~Game();

    Game(Game const&) = delete;
    Game& operator=(Game const&) = delete;

    /// @brief Update game state.
    void update();

    /// @brief Check if the game is still running.
    /// @return 
    bool isRunning() const;

    /// @brief Handle a window resize event.
    /// @param width 
    /// @param height 
    void onResize(uint32_t width, uint32_t height);

private:
    bool                                m_running       = false;
    GLFWwindow*                         m_pWindow       = nullptr;
    core::Timer                         m_frameTimer    = {};
    std::shared_ptr<gfx::RenderBackend> m_renderbackend = {};

    // TEMP rendering stuff before rendering abstraction is added
    WGPUTexture                         m_depthStencilTarget = nullptr;
    WGPUTextureView                     m_depthStencilTargetView = nullptr;

    WGPUBindGroupLayout                 m_sceneDataBindGroupLayout = nullptr;
    WGPUBindGroupLayout                 m_objectDataBindGroupLayout = nullptr;
    WGPUPipelineLayout                  m_pipelineLayout = nullptr;
    WGPURenderPipeline                  m_pipeline = nullptr;
};
