#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// @brief Available camera types.
enum class CameraType
{
	Perspective,
	Orthographic,
};

/// @brief Perspective camera params.
struct PerspectiveCamera
{
	float yFOV			= 60.0F;
	float aspectRatio	= 1.0F;
	float zNear			= 0.1F;
	float zFar			= 1000.0F;
};

/// @brief Orthographic camera params.
struct OrthographicCamera
{
	float xMag	= 1.0F;
	float yMag	= 1.0F;
	float zNear = 0.1F;
	float zFar	= 1000.0F;
};

/// @brief Camera component to render a scene.
struct Camera
{
public:
	/// @brief Create a perspective camera.
	/// @param perspective 
	Camera(PerspectiveCamera const& perspective);

	/// @brief Create an orhographic camera.
	/// @param ortho 
	Camera(OrthographicCamera const& ortho);

	/// @brief Default destructor.
	~Camera() = default;

	Camera(Camera const&) = delete;
	Camera& operator=(Camera const&) = delete;

	/// @brief Retrieve this camera's projection matrix.
	/// @return 
	glm::mat4 matrix() const;

public:
	CameraType type = CameraType::Perspective;
	union CameraParams 
	{
		PerspectiveCamera perspective;
		OrthographicCamera ortho;
	} params;
};
