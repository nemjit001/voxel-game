#pragma once

#include <memory>

#include "rendering/material.hpp"
#include "rendering/mesh.hpp"

/// @brief Render component that specifies render data for an entity.
class RenderComponent
{
public:
	std::shared_ptr<gfx::Mesh>		mesh = {};
	std::shared_ptr<gfx::Material>	material = {};
};
