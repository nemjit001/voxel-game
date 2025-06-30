#pragma once

#include <memory>
#include <GLFW/glfw3.h>

#include "core/timer.hpp"
#include "core/world.hpp"
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

    /// @brief Handle a window resize event.
    /// @param width 
    /// @param height 
    void onResize(uint32_t width, uint32_t height);

    /// @brief Check if the game is still running.
    /// @return 
    bool isRunning() const;

private:
    bool                            m_running       = false;
    GLFWwindow*                     m_pWindow       = nullptr;
    Timer                           m_frameTimer    = {};

    std::shared_ptr<RenderBackend>  m_renderbackend = {};
    std::shared_ptr<World>          m_world         = {};
};
