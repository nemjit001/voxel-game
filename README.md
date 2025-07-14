# Voxel Game

This is a voxel game written in C++17, made mostly from scratch with some convenience libraries.
The game uses WebGPU for rendering, GLFW for windowing and input, and the EnTT ECS for representing game entities and game state.

## Building the game

The game should build out-of-the-box using CMake (version ^3.25). All dependencies are managed using CMake FetchContent and should be fetched
during configuration.

## Dependencies

A complete dependency list for the game is given below:

| Name        | Version   | Reason                     |
| ----------- | --------- | -------------------------- |
| EnTT        | v3.15.0   | ECS                        |
| GLFW        | v3.4      | Windowing handling & input |
| GLM         | Latest    | Linear algebra             |
| SPDLog      | v1.15.3   | Fast logging               |
| STB         | Latest    | Image asset loading        |
| TinyGLTF    | v2.9.6    | Mesh asset loading         |
| WebGPU      | Varying   | Graphics API               |
| GLFW3WebGPU | v1.2.0    | GLFW3 WebGPU integration   |

## Platforms

The table below gives platform support indications, some platforms are untested.

| Platform    | Support      |
| ----------- | ------------ |
| Windows x64 | Full support |
| Linux x64   | Full support |
| MacOS       | Untested     |
| WebAssembly | Full support |

## Licensing

The game and its assets are licensed under the MIT license, meaning it is provided as is with no warranty.

