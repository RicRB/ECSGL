#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
struct Transform {
private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 matrix;
	bool cached = false; // if the matrix is cached, we don't need to recalculate it every time
	bool cachingEnabled = true; // if caching is enabled, we will cache the matrix
public:
	Transform(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f))
		:  position(position), rotation(rotation), scale(scale) {
	}
	~Transform() {}
	const glm::mat4 getMatrix()
	{
		if (cachingEnabled && cached) return matrix;
		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, position);
		mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		mat = glm::scale(mat, scale);
		if (cachingEnabled)
		{
			matrix = mat;
			cached = true;
		}
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
	void setPosition(const glm::vec3& pos) {
		position = pos;
		cached = false; // invalidate cache
	}
	void setRotation(const glm::vec3& rot) {
		rotation = rot;
		cached = false; // invalidate cache
	}
	void setScale(const glm::vec3& scl) {
		scale = scl;
		cached = false; // invalidate cache
	}
	glm::vec3& getPositionReference() {
		return position;
	}
	glm::vec3& getRotationReference() {
		return rotation;
	}

	glm::vec3 getPosition() const {
		return position;
	}
	glm::vec3 getRotation() const {
		return rotation;
	}
	glm::vec3 getScale() const {
		return scale;
	}
	const glm::vec3 getForward() const {
		// Convert degrees to radians if needed
		float pitch = glm::radians(rotation.x);
		float yaw = glm::radians(rotation.y);

		glm::vec3 forward;
		forward.x = cos(pitch) * sin(yaw);
		forward.y = sin(pitch);
		forward.z = -cos(pitch) * cos(yaw); // OpenGL forward is -Z

		return glm::normalize(forward);
	}
	const glm::vec3 getRight() const {
		float pitch = glm::radians(rotation.x);
		float yaw = glm::radians(rotation.y);

		glm::vec3 forward;
		forward.x = cos(pitch) * sin(yaw);
		forward.y = sin(pitch);
		forward.z = -cos(pitch) * cos(yaw);
		forward = glm::normalize(forward);

		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));
		return right;
	}
	void setCaching(bool enable) {
		cachingEnabled = enable;
		if (!enable) {
			cached = false; // invalidate cache if caching is disabled
		}
	}
};