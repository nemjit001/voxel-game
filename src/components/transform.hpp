#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

/// @brief World transform component to give entities a position in this world.
class Transform
{
public:
	/// @brief Calculate the world-space affine transformation matrix for this transform.
	/// @return 
	inline glm::mat4 matrix() const;

	/// @brief Retrieve the transform's forward vector.
	/// @return 
	inline glm::vec3 forward() const;

	/// @brief Retrieve the transform's up vector.
	/// @return 
	inline glm::vec3 up() const;

	/// @brief Retrieve the transform's right vector.
	/// @return 
	inline glm::vec3 right() const;

public:
	static constexpr glm::vec3 WORLD_ORIGIN		= { 0.0F, 0.0F, 0.0F }; // World origin
	static constexpr glm::vec3 WORLD_FORWARD	= { 0.0F, 0.0F, 1.0F }; // World forward vector
	static constexpr glm::vec3 WORLD_UP			= { 0.0F, 1.0F, 0.0F }; // World up vector
	static constexpr glm::vec3 WORLD_RIGHT		= { 1.0F, 0.0F, 0.0F }; // World right vector

	glm::vec3 position	= { 0.0F, 0.0F, 0.0F };
	glm::quat rotation	= { 1.0F, 0.0F, 0.0F, 0.0F };
	glm::vec3 scale		= { 1.0F, 1.0F, 1.0F };
};

glm::mat4 Transform::matrix() const
{
	return glm::translate(glm::identity<glm::mat4>(), position)
		* glm::mat4_cast(rotation)
		* glm::scale(glm::identity<glm::mat4>(), scale);
}

glm::vec3 Transform::forward() const
{
	glm::vec4 const forward = glm::mat4_cast(rotation) * glm::vec4(WORLD_FORWARD, 0.0F);
	return glm::normalize(glm::vec3(forward));
}

glm::vec3 Transform::up() const
{
	glm::vec4 const up = glm::mat4_cast(rotation) * glm::vec4(WORLD_UP, 0.0F);
	return glm::normalize(glm::vec3(up));
}

glm::vec3 Transform::right() const
{
	glm::vec4 const right = glm::mat4_cast(rotation) * glm::vec4(WORLD_RIGHT, 0.0F);
	return glm::normalize(glm::vec3(right));
}
