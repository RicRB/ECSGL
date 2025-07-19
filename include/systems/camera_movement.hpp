#include "ecs/ecs.hpp"
#include "engine.h"

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
	void update(const float deltaTime) override {
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
	}
	void cleanup() override {
		// Cleanup code if needed
	}
	const char* getName() const override {
		return "CameraMovementSystem";
	}
};