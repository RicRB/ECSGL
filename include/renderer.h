#pragma once
#include "ecs/ecs_system.h"
#include "transform.h"
#include "gl_def.h"
#include "material.h"
#include "mesh.h"
#include "engine.h"
#include "renderer/graphics_api.h"
class Renderer : public System
{
	View<Transform, Mesh, Material> currentView;
	GraphicsAPI* graphicsAPI = nullptr;
	
public:
	Renderer();
	~Renderer();
	void Start(Registry& registry) override {};
	void Update(Registry& registry) override;
	void Run();
	void DrawElements(Registry& registry);
	void Clear() const;
	inline void DrawEntity(const Entity entity);
};