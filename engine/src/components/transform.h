#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
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
		const glm::vec3 eulerRad = glm::radians(rotation);

		// 2) Build a quaternion from Euler (normalized for safety)
		// Option A (simple):
		glm::quat q = glm::normalize(glm::quat(eulerRad));
		glm::mat4 quatRotation = glm::toMat4(q);

		glm::mat4 mat = glm::translate(glm::mat4(1.0f), position) * quatRotation * glm::scale(glm::mat4(1.0f), scale);
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
		// 1) Build rotation from Euler *in radians*
		const glm::vec3 e = glm::radians(rotation);   // rotation is in degrees
		glm::quat q = glm::normalize(glm::quat(e));   // safe: normalize

		// 2) Rotation matrix and its transpose (inverse for orthonormal)
		const glm::mat3 R = glm::mat3_cast(q);
		const glm::mat3 Rt = glm::transpose(R);

		// 3) Compose view = R^T * T^{-1}
		glm::mat4 V(1.0f);
		V[0] = glm::vec4(Rt[0], 0.0f);                // columns
		V[1] = glm::vec4(Rt[1], 0.0f);
		V[2] = glm::vec4(Rt[2], 0.0f);
		V[3] = glm::vec4(-Rt * position, 1.0f);       // translation part
		return V;
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
		glm::quat q = glm::normalize(glm::quat(glm::radians(rotation)));
		// In GLM, -Z is the conventional forward in right-handed systems
		return glm::normalize(q * glm::vec3(0.0f, 0.0f, -1.0f));
	}

	const glm::vec3 getRight() const {
		glm::quat q = glm::normalize(glm::quat(glm::radians(rotation)));
		return glm::normalize(q * glm::vec3(1.0f, 0.0f, 0.0f));
	}

	const glm::vec3 getUp() const {
		glm::quat q = glm::normalize(glm::quat(glm::radians(rotation)));
		return glm::normalize(q * glm::vec3(0.0f, 1.0f, 0.0f));
	}
	void setCaching(bool enable) {
		cachingEnabled = enable;
		if (!enable) {
			cached = false; // invalidate cache if caching is disabled
		}
	}
};