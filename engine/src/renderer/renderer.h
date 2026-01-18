#pragma once
#include "components/transform.h"
#include "core/resource_manager.h"
class Renderer
{
public:
private:
	float fps = 0.0f;
	float times = 0.0f;
	
public:
	Renderer();
	~Renderer();
	void Start(Registry& registry) {};
	void Update(Registry& registry) {};
	void Run();
	void DrawElements(Registry& registry, ResourceManager& resourceManager, Entity camera);
	void Clear() const;
};