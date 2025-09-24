#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
struct Camera
{
	glm::mat4 projection;
	Camera(glm::mat4 projection = glm::perspective(glm::radians(45.0f), 960.0f / 540.0f, 0.1f, 100.0f))
		: projection(projection) {}
	~Camera() {}
};