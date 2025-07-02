#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace components
{
	/// @brief Camera component to render a scene.
	struct Camera
	{
	public:
		inline glm::mat4 matrix() const;

	public:
		float yFOV			= 60.0F;
		float aspectRatio	= 1.0F;
		float zNear			= 0.1F;
		float zFar			= 1000.0F;
	};

	glm::mat4 Camera::matrix() const
	{
		return glm::perspective(glm::radians(yFOV), aspectRatio, zNear, zFar);
	}
}
