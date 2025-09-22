#pragma once
#include "ecs_def.h"
#include <cstdint>
#include <vector>
struct Entity
{
	uint32_t entityID = ECS_INVALID_ID;
	uint32_t version = 0;
	Entity(uint32_t ID, uint32_t version)
		: entityID(ID), version(version)
	{}
	Entity()
	{}
};
struct HierarchyData
{
	uint32_t parent = ECS_INVALID_ID;
	std::vector<uint32_t> children;
	void removeChild(uint32_t childID)
	{
		children.erase(std::remove(children.begin(), children.end(), childID), children.end());
	}
};