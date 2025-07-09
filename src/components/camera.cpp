#include "camera.hpp"

#include <cassert>

Camera::Camera(PerspectiveCamera const& perspective)
	:
	type(CameraType::Perspective),
	params{}
{
	params.perspective = perspective;
}

Camera::Camera(OrthographicCamera const& ortho)
	:
	type(CameraType::Orthographic),
	params{}
{
	params.ortho = ortho;
}

glm::mat4 Camera::matrix(float aspectRatio) const
{
	if (type == CameraType::Perspective)
	{
		PerspectiveCamera const camera = params.perspective;
		return glm::perspective(glm::radians(camera.yFOV), aspectRatio, camera.zNear, camera.zFar);
	}
	else
	{
		OrthographicCamera const camera = params.ortho;
		float const halfY = camera.size * 0.5F;
		float const halfX = halfY * aspectRatio;

		return glm::ortho(-halfX, halfX, -halfY, halfY, camera.zNear, camera.zFar);
	}

	// Unreachable code
	assert(false && "Unreachable code reached");
	return glm::identity<glm::mat4>();
}
