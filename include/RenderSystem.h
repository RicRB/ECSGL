#pragma once
#include "ECS/ECS.hpp"
#include "Transform.h"
#include "GLDef.h"
#include "Material.h"
#include "Mesh.h"
#include "GameEngine.h"

class RenderSystem : public System
{
	View<Transform, Mesh, Material> currentView;
	
public:
	RenderSystem();
	~RenderSystem();
	void Start() override {};
	void Update() override {};
	void Run();
	void DrawElements();
	void Clear() const;
	inline void DrawEntity(const Entity entity);
};