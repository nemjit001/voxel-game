#include "transform.hpp"

void Transform::lookAt(glm::vec3 const& forward, glm::vec3 const& up)
{
	rotation = glm::quatLookAt(forward, up);
}
