#pragma once
#include "ecs/ecs.hpp"
#include "physics_body.h"
#include "transform.h"
class PhysicsSystem : public System
{
private:
	
	void Update(Registry& registry, const double deltaTime) override;

	void MoveBody(Transform transform, PhysicsBody body);
};