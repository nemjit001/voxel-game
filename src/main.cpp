#if     _WIN32
#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif  // !NDEBUG
#endif  // _WIN32

#include <stdexcept>
#include <spdlog/spdlog.h>

#include "macros.hpp"

#if     GAME_PLATFORM_EMSCRIPTEN
    #include <emscripten/emscripten.h>
#endif  // GAME_PLATFORM_EMSCRIPTEN

#include "game.hpp"

int main(int argc, char** argv)
{
#if     _WIN32
#ifndef NDEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif  // !NDEBUG
#endif  // _WIN32

    (void)(argc);
    (void)(argv);

    try
    {
        // Create game
        Game game{};
#if     GAME_PLATFORM_EMSCRIPTEN
        // Enter emscripten update loop
        emscripten_set_main_loop_arg([](void* pUserData) {
            Game* pGame = reinterpret_cast<Game*>(pUserData);
            pGame->update();
        }, &game, 0, true);
#else
        // Enter default platform update loop
        while (game.isRunning()) {
            game.update();
        }
#endif  // GAME_PLATFORM_EMSCRIPTEN
    }
    catch (std::exception& e)
    {
        SPDLOG_ERROR("Fatal exception: {}", e.what());
    }

    return 0;
}
