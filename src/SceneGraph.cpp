#include "Registry.hpp"
void SceneGraph::ParentEntity(Entity child, Entity parent)
{
	_ParentEntity(child, parent);
}

Entity SceneGraph::GetParent(Registry& registry, Entity child)
{
	auto entities = registry.GetActiveEntities();
	return sceneGraph[child.entityID].parent == ECS_INVALID_ID ? Entity() : entities[sceneGraph[child.entityID].parent];
}
std::vector<Entity> SceneGraph::GetChildren(Registry& registry, Entity parent)
{
	auto entities = registry.GetActiveEntities();
	std::vector<Entity> children;
	for (auto entityID : sceneGraph[parent.entityID].children)
	{
		children.push_back(entities[entityID]);
	}
	return children;
}
glm::mat4x4 SceneGraph::GetLocalToWorldMatrix(Registry& registry, Entity child)
{
	return _GetParentWorldMatrix(registry, child);
}

void SceneGraph::_ParentEntity(Entity child, Entity parent)
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
glm::mat4x4 SceneGraph::_GetParentWorldMatrix(Registry& registry, Entity child)
{
	Entity parent = GetParent(registry, child);

	// parent doesnt exist
	if (parent.entityID == ECS_INVALID_ID)
	{
		return glm::mat4(1.0f);
	}
	glm::mat4x4 parentMatrix = registry.GetComponent<Transform>(parent).matrix();
	return  _GetParentWorldMatrix(registry, parent) * parentMatrix;
}