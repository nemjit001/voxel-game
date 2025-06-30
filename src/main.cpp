#include <stdexcept>
#include <spdlog/spdlog.h>

#include "game.hpp"

int main(int argc, char** argv)
{
    (void)(argc);
    (void)(argv);

    try
    {
        Game game{};
        while (game.isRunning()) {
            game.update();
        }
    }
    catch (std::exception& e)
    {
        SPDLOG_ERROR("Fatal exception: {}", e.what());
    }

    return 0;
}
