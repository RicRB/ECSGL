#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transform(glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f))
		: position(position), rotation(rotation), scale(scale) {}
	~Transform() {}
	const glm::mat4 matrix() const
	{
		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, position);
		mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		mat = glm::scale(mat, scale);
		return mat;
	}
	// returns without the scale
	const glm::mat4 viewMatrix() const
	{
		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, position);
		mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		return mat;
	}
};