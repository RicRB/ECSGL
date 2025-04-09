#pragma once
#include "ECSDef.h"
#include <cstdint>
#include <list>
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
	Entity parent;
	std::list<Entity> children;
};