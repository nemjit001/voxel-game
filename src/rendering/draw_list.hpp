#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "mesh.hpp"

namespace gfx
{
    /// @brief Draw command with dynamic offsets and associated mesh.
    struct DrawCommand
    {
        uint32_t                    cameraOffset;
        uint32_t                    materialOffset;
        uint32_t                    objectOffset;
        std::shared_ptr<gfx::Mesh>  mesh;
    };

    /// @brief Draw list containing per-pass draw calls sorted by pass name.
    class DrawList
    {
    public:
        /// @brief Append a draw command to a render pass.
        /// @param pass Pass name to append draw call to.
        /// @param command Draw command containing draw data.
        void append(std::string const& pass, DrawCommand const& command);

        std::vector<DrawCommand> commands(std::string const& pass) const;

    private:
        std::unordered_map<std::string, std::vector<DrawCommand>> m_commands{};
    };
} // namespace gfx
