#pragma once
#include "entity.h"
#include "components/transform.h"
#include <array>
#include "glm/glm.hpp"
#include <iostream>
class Registry;
class SceneGraph
{
public:

private:
	std::unique_ptr<std::array<HierarchyData, MAX_ENTITIES>> sceneGraph;
public:
	SceneGraph();
	~SceneGraph() = default;
	/**
	* @brief sets the parent of the first entity to the second entity. leave second parameter empty for no parent
	**/
	void ParentEntity(Entity child, Entity parent = Entity());
	Entity GetParent(Registry& registry, Entity child);
	std::vector<Entity> GetChildren(Registry& registry, Entity parent);
	glm::mat4x4 GetLocalToWorldMatrix(Registry& registry, Entity child);
private:



	SceneGraph(SceneGraph const&) = delete;
	void operator=(SceneGraph const&) = delete;

	void _ParentEntity(Entity child, Entity parent = Entity());

	// recursive... ik.. its bad but man its a scene graph
	glm::mat4x4 _GetParentWorldMatrix(Registry& registry, Entity child);
};