#pragma once
#include "ecs/ecs.hpp"
#include "components/lights.h"
#include "components/camera.h"
#include "renderer/renderer.h"
#include "renderer/compute_dispatcher.h"
#include "renderer/compute_manager.h"
#include <memory>
#include "window.h"
#include "renderer/graphics_api.h"
class Engine
{
public:
	//std::unique_ptr<Scene> currentScene;
	Registry registry;
	ResourceManager resourceManager;
	Renderer renderer;
	std::unique_ptr<GraphicsAPI> graphicsAPI;
	GraphicsAPIType graphicsAPIType;
	std::unique_ptr<ComputeManager>* computeManager;
	std::unique_ptr<BufferManager>* bufferManager;
	Shader defaultShader;
	Material defaultMaterial;
	Mesh defaultMesh;
	Camera currentCamera;
	Entity currentCameraEntity;
	std::weak_ptr<Window> window;
	
public:
	Engine(const GraphicsAPIType graphicsAPIType);
	void initialize(const std::shared_ptr<Window>& window);
	void Start();
	void Update();
	void LoadScene(const std::string& scenePath);
	void SetCamera(Entity camera);
	void cleanup();
	Scene GetScene();
	~Engine();
private:

	Engine(const Engine&) = delete;
};
// do an if guard

namespace engine
{
	namespace internal
	{
		extern Engine* _currentEnginePtr;
	}
	inline void SetInstance(Engine* engineInstance);
	inline Engine& GetEngine();
	Entity CreateEmptyEntity();
	Entity CreateDefaultObject(Transform transform = Transform());
	Entity CreateObject(Transform transform = Transform(), Mesh mesh = {}, Material material = {});
	Entity CreateDirLight(const glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f), const glm::vec3 ambient = glm::vec3(0.0f), const glm::vec3 diffuse = glm::vec3(1.0f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 2.0f);
	Entity CreatePointLight(const Transform transform = Transform(), const glm::vec3 ambient = glm::vec3(0.0f), const glm::vec3 diffuse = glm::vec3(1.0f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 1.0f, const float constant = 1.0f, const float linear = 0.09f, const float quadratic = 0.032f);
	void SetCamera(Entity entity);
	const Entity GetCameraEntity();
	Camera GetCamera();
	Registry& GetRegistry();
};
namespace graphics
{
	std::unique_ptr<GraphicsAPI>& GetGraphicsAPI();
	GraphicsAPIType getGraphicsAPIType();
}