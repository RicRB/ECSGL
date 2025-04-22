#pragma once
#include "ECS/ECS.hpp"
#include "Transform.h"
#include <array>
#include "glm/glm.hpp"
#include <iostream>
class Scenegraph
{

public:

private:
	static Scenegraph instance;
	std::array<HierarchyData, MAX_ENTITIES> sceneGraph;
public:

	/**
	* @brief sets the parent of the first entity to the second entity. leave second parameter empty for no parent
	**/
	static void ParentEntity(Entity child, Entity parent = Entity())
	{
		GetInstance()._ParentEntity(child, parent);
	}
	static Entity GetParent(Entity child)
	{
		auto entities = Registry::GetInstance().GetActiveEntities();
		return GetInstance().sceneGraph[child.entityID].parent == ECS_INVALID_ID ? Entity() : entities[GetInstance().sceneGraph[child.entityID].parent];
	}
	static std::vector<Entity> GetChildren(Entity parent)
	{
		auto entities = Registry::GetInstance().GetActiveEntities();
		std::vector<Entity> children;
		for (auto entityID : GetInstance().sceneGraph[parent.entityID].children)
		{
			children.push_back(entities[entityID]);
		}
		return children;
	}
	static glm::mat4x4 GetLocalToWorldMatrix(Entity child)
	{
		return GetInstance()._GetParentWorldMatrix(child);
	}
private:

	static Scenegraph& GetInstance()
	{
		static Scenegraph instance;
		return instance;
	}

	Scenegraph() = default;
	~Scenegraph() = default;
	Scenegraph(Scenegraph const&) = delete;
	void operator=(Scenegraph const&) = delete;

	void _ParentEntity(Entity child, Entity parent = Entity())
	{
		if (parent.entityID == child.entityID)
		{
			std::cout << "Entity ID is the same" << "\n";
			return;
		}

		HierarchyData* childData = &sceneGraph[child.entityID];
		// remove from previous parent child list
		if (childData->parent != ECS_INVALID_ID)
		{
			sceneGraph[childData->parent].removeChild(child.entityID);
		}
		// add to parent's child list
		if (parent.entityID != ECS_INVALID_ID)
		{
			HierarchyData* parentData = &sceneGraph[parent.entityID];
			if (parentData->parent == child.entityID)
			{
				std::cout << "Parent cannot be child" << "\n";
				return;
			}

			sceneGraph[parent.entityID].children.push_back(child.entityID);
		}
		// parent
		childData->parent = parent.entityID;
	}

	// recursive... ik.. its bad but man its a scene graph
	glm::mat4x4 _GetParentWorldMatrix(Entity child)
	{
		Entity parent = GetParent(child);

		// parent doesnt exist
		if (parent.entityID == ECS_INVALID_ID)
		{
			return glm::mat4(1.0f);
		}
		glm::mat4x4 parentMatrix = Registry::GetInstance().GetComponent<Transform>(parent).matrix();
		return  _GetParentWorldMatrix(parent) * parentMatrix;
	}
};