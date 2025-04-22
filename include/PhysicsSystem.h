#pragma once
#include "ECS/ECS.hpp"
#include "PhysicsBody.h"
#include "Transform.h"
class PhysicsSystem : public System
{
private:
	
	void Update(Registry& registry, const double deltaTime) override;

	void MoveBody(Transform transform, PhysicsBody body);
};