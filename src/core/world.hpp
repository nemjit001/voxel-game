#pragma once

#include <entt/entt.hpp>

/// @brief The World class contains entities and manages the game world state.
class World
{
public:
	World() = default;
	~World() = default;

	World(World const&) = delete;
	World& operator=(World const&) = delete;

	/// @brief Retrieve the world's internal ECS registry.
	/// @return The internal ECS registry for this world.
	entt::registry&			getRegistry()		{ return m_registry; }
	entt::registry const&	getRegistry() const { return m_registry; }

private:
	entt::registry m_registry = {};
};
