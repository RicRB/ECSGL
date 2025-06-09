#pragma once
#include "ecs/ecs.hpp"
#include "transform.h"
#include "gl_def.h"
#include "material.h"
#include "mesh.h"
#include "engine.h"

class Renderer : public System
{
	View<Transform, Mesh, Material> currentView;
	
public:
	Renderer();
	~Renderer();
	void Start(Registry& registry) override {};
	void Update(Registry& registry, const double deltaTime) override;
	void Run();
	void DrawElements(Registry& registry);
	void Clear() const;
	inline void DrawEntity(const Entity entity);
};