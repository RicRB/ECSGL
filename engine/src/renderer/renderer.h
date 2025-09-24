#pragma 
#include "components/transform.h"
#include "renderer/graphics_api.h"
#include "core/resource_manager.h"
class Renderer
{
public:
	std::unique_ptr<GraphicsAPI> graphicsAPI;
	GraphicsAPIType rendererType;
private:
	View<Transform, Mesh, Material> currentView;
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
	inline void DrawEntity(const Entity entity);
};