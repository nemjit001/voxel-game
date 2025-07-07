#include "camera.hpp"

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

glm::mat4 Camera::matrix() const
{
	if (type == CameraType::Perspective)
	{
		PerspectiveCamera const camera = params.perspective;
		return glm::perspective(glm::radians(camera.yFOV), camera.aspectRatio, camera.zNear, camera.zFar);
	}
	else
	{
		OrthographicCamera const camera = params.ortho;
		float const halfX = camera.xMag * 0.5F;
		float const halfY = camera.yMag * 0.5f;

		return glm::ortho(-halfX, halfX, -halfY, halfY, camera.zNear, camera.zFar);
	}

	// Unreachable code
	return glm::identity<glm::mat4>();
}
