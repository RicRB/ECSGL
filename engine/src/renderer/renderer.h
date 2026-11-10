#pragma once
#include "components/transform.h"
#include "core/resource_manager.h"
class Renderer
{
public:
private:
	std::vector<MeshData*> meshes;
	std::vector<MaterialData*> materials;
	std::vector<glm::mat4x4> modelMatrices;
public:
	Renderer();
	~Renderer();
	void Start(Registry& registry) {};
	void Update(Registry& registry) {};
	void Run();
	void DrawElements(Registry& registry, ResourceManager& resourceManager, Entity camera);
	void Clear() const;
};