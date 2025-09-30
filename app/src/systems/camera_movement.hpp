#include "ecs/ecs.hpp"
#include "core/engine.h"
#include "core/time.h"
#include "core/input.h"
class CameraMovementSystem : public System {
public:
	float moveSpeed = 5.0f; // Speed of camera movement
	CameraMovementSystem() = default;
	void start() override {
		// Initialization code if needed
		auto camera = engine::GetCameraEntity();
		auto& registry = engine::GetRegistry();
		registry.GetComponent<Transform>(camera).setCaching(false); // Disable caching for camera transform to ensure real-time updates
	}
	void update() override {
		auto deltaTime = (float)Time::getDeltaTime();
		auto camera = engine::GetCameraEntity();
		auto& registry = engine::GetRegistry();
		auto& transform = registry.GetComponent<Transform>(camera);
		// Example movement logic
		if (Input::GetKeyPressed(Key::W)) {
			transform.getPositionReference() += transform.getForward() * deltaTime * moveSpeed;
		}
		if (Input::GetKeyPressed(Key::S)) {
			transform.getPositionReference() -= transform.getForward() * deltaTime * moveSpeed;
		}
		if (Input::GetKeyPressed(Key::A)) {
			transform.getPositionReference() -= transform.getRight() * deltaTime * moveSpeed;
		}
		if (Input::GetKeyPressed(Key::D)) {
			transform.getPositionReference() += transform.getRight() * deltaTime * moveSpeed;
		}
		if (Input::GetKeyPressed(Key::Space)) {
			transform.getPositionReference() += transform.getUp() * deltaTime * moveSpeed;
		}
		if (Input::GetKeyPressed(Key::LeftControl)) {
			transform.getPositionReference() -= transform.getUp() * deltaTime * moveSpeed;
		}
		if (Input::GetKeyPressed(Key::Right)) {
			transform.getRotationReference().y -= 90.0f * deltaTime; // Rotate right
		}
		if (Input::GetKeyPressed(Key::Left)) {
			transform.getRotationReference().y += 90.0f * deltaTime; // Rotate left
		}
		if (Input::GetKeyPressed(Key::Up)) {
			transform.getRotationReference().x += 90.0f * deltaTime; // Rotate up
		}
		if (Input::GetKeyPressed(Key::Down)) {
			transform.getRotationReference().x -= 90.0f * deltaTime; // Rotate down
		}
	}
	void cleanup() override {
		// Cleanup code if needed
	}
	const char* getName() const override {
		return "CameraMovementSystem";
	}
};