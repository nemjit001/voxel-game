#include "game.hpp"

#include <cassert>
#include <spdlog/spdlog.h>

#include "macros.hpp"
#include "core/files.hpp"
#include "assets/mesh_loader.hpp"
#include "assets/texture_loader.hpp"
#include "components/camera.hpp"
#include "components/render_component.hpp"
#include "components/transform.hpp"

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

    (void)(pThat);
    (void)(keycode);
    (void)(scancode);
    (void)(action);
    (void)(mods);
}

static void mousePosCallback(GLFWwindow* pWindow, double xpos, double ypos)
{
    Game* pThat = reinterpret_cast<Game*>(glfwGetWindowUserPointer(pWindow));
    assert(pThat != nullptr);

    (void)(pThat);
    (void)(xpos);
    (void)(ypos);
}

Game::Game()
{
    // Set up default logger state
#if     GAME_BUILD_TYPE_DEBUG
    spdlog::set_level(spdlog::level::trace);
#endif

    // Dump some basic info
    SPDLOG_INFO("Initial window size: {} x {}", DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    SPDLOG_INFO("Program directory: {}", core::fs::getProgramDirectory());
#if     GAME_BUILD_TYPE_DEBUG
    SPDLOG_WARN("Running Debug build!");
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

    // Initialize game systems
    SPDLOG_INFO("Initializing game systems");
    m_registry = std::make_unique<entt::registry>();
    m_renderer = std::make_unique<Renderer>(m_renderbackend);

    // Set up simple game world with basic meshes / camera for now
    {
        // Set up a simple camera position w/ lookat to world origin
        auto cameraTransform = Transform{ { 0.0F, 2.0F, 5.0F } };
        cameraTransform.lookAt(glm::normalize(Transform::WORLD_ORIGIN - cameraTransform.position));

        // Create camera entity
        auto camera = m_registry->create();
        m_registry->emplace<Camera>(camera, PerspectiveCamera{ 60.0F, 0.1F, 1000.0F });
        m_registry->emplace<Transform>(camera, cameraTransform);

        // Set up 2 entities with render components using a mesh file loaded from disk
        auto suzanneMesh = assets::MeshLoader().load(core::fs::getFullAssetPath("assets/suzanne.glb"));
        auto suzanneMaterial = std::make_shared<gfx::Material>();
        suzanneMaterial->albedoTexture = assets::TextureLoader().load(core::fs::getFullAssetPath("assets/brickwall.jpg"), gfx::TextureMode::ColorData);
        suzanneMaterial->normalTexture = assets::TextureLoader().load(core::fs::getFullAssetPath("assets/brickwall_normal.jpg"), gfx::TextureMode::NonColorData);

        auto suzanne1 = m_registry->create();
        m_registry->emplace<RenderComponent>(suzanne1, RenderComponent{ suzanneMesh, suzanneMaterial });
        m_registry->emplace<Transform>(suzanne1, Transform{ { 2.0F, 0.0F, 0.0F } });
        
        auto suzanne2 = m_registry->create();
        m_registry->emplace<RenderComponent>(suzanne2, RenderComponent{ suzanneMesh, suzanneMaterial });
        m_registry->emplace<Transform>(suzanne2, Transform{ { -2.0F, 0.0F, 0.0F } });
    }

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

    // Handle platform events
    glfwPollEvents();

    // Handle system updates
    //

    // Render grame frame if not minimized
    if (m_windowVisible) {
        m_renderer->render(*m_registry);
    }
}

void Game::onResize(uint32_t width, uint32_t height)
{
    // Check if window still has a valid size
    m_windowVisible = (width != 0 && height != 0);

    // Only forward resize if window is still visible (i.e. not minimized)
    if (m_windowVisible)
    {
        SPDLOG_INFO("Window resized ({} x {})", width, height);
        m_renderer->onResize(width, height);
    }
}

bool Game::isRunning() const
{
    return m_running && !glfwWindowShouldClose(m_pWindow);
}
