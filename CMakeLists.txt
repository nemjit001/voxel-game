cmake_minimum_required(VERSION 3.25)
project("VoxelGame" VERSION 0.1.0)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include("cmake/utils.cmake")
include(FetchContent)

FetchContent_Declare(entt
    GIT_REPOSITORY  https://github.com/skypjack/entt.git
    GIT_TAG         v3.15.0
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw.git
    GIT_TAG         3.4
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog.git
    GIT_TAG         v1.15.3
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(stb
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(webgpu
    GIT_REPOSITORY  https://github.com/eliemichel/WebGPU-distribution.git
    GIT_TAG         main-v0.2.0
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(glfw3webgpu
    GIT_REPOSITORY  https://github.com/eliemichel/glfw3webgpu.git
    GIT_TAG         v1.2.0
    GIT_SHALLOW     TRUE
)

FetchContent_MakeAvailable(entt glfw glm spdlog stb webgpu glfw3webgpu)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")

add_executable(VoxelGame
    "src/main.cpp"
    "src/game.cpp"
    "src/game.hpp"
    "src/core/files.cpp"
    "src/core/files.hpp"
    "src/core/timer.hpp"
    "src/rendering/mesh.cpp"
    "src/rendering/mesh.hpp"
    "src/rendering/render_backend.cpp"
    "src/rendering/render_backend.hpp"
    "src/rendering/texture.cpp"
    "src/rendering/texture.hpp"
    "src/rendering/vertex_layout.hpp"
    "src/components/camera.cpp"
    "src/components/camera.hpp"
    "src/components/render_component.hpp"
    "src/components/transform.hpp"
    "src/systems/renderer.cpp"
    "src/systems/renderer.hpp"
)
target_compile_features(VoxelGame PRIVATE cxx_std_17)
target_include_directories(VoxelGame PRIVATE "src/")
target_link_libraries(VoxelGame PRIVATE EnTT::EnTT glfw glm::glm spdlog stb webgpu glfw3webgpu)
target_compile_definitions(VoxelGame PRIVATE GLM_FORCE_RADIANS GLM_FORCE_RIGHT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_enable_extended_warnings(VoxelGame)
target_copy_webgpu_binaries(VoxelGame)
target_register_assets(VoxelGame
    "assets/shaders/shaders.wgsl"
)
