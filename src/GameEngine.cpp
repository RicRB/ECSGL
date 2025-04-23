#include "GameEngine.h"
#include <iostream>
#include <filesystem>


Engine::Engine()
{
}
void Engine::Start()
{
	std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
	defaultShader = resourceManager.LoadShader("res/shaders/Basic.vert", "res/shaders/Basic.frag");
	resourceManager.InitShaders(registry);
	defaultMaterial = Material(*defaultShader.rendererID);
	defaultMesh = resourceManager.LoadOBJ("res/models/cube.obj");
	printf("Game started");
}

void Engine::Update(const double deltaTime)
{
	
}

void Engine::LoadScene(const std::string& scenePath)
{
	// parse scene
	// load scene
}

Scene Engine::GetScene()
{
	return Scene();
}




namespace GameEngine {
	Engine* currentEngine = nullptr;
	void GameEngine::SetCurrentEngine(Engine& engine)
	{
		currentEngine = &engine;
	}

	Entity GameEngine::CreateEmptyEntity()
	{
		Entity entity = currentEngine->registry.CreateEntity();
		Registry& reg = currentEngine->registry;
		return entity;
	}
	Entity GameEngine::CreateObject(Transform transform, Mesh mesh, Material material)
	{
		if (mesh.indexID == 0)
		{
			mesh = currentEngine->defaultMesh;
		}
		if (material.shaderID == 0)
		{
			material = currentEngine->defaultMaterial;
		}
		Registry& registry = currentEngine->registry;
		Entity entity = registry.CreateEntity();
		transform.position = glm::vec3(1.0f);
		registry.AssignComponent(entity, transform);
		registry.AssignComponent(entity, mesh);
		registry.AssignComponent(entity, material);
		return entity;
	}

	Entity GameEngine::CreateDirLight(const glm::vec3 direction, const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity)
	{
		Registry& registry = currentEngine->registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, DirectionalLight(direction, ambient, diffuse, specular, intensity));
		return entity;
	}

	Entity GameEngine::CreatePointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity, const float constant, const float linear, const float quadratic)
	{
		Registry& registry = currentEngine->registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, PointLight(ambient, diffuse, specular, intensity, constant, linear, quadratic));
		return entity;
	}
	void GameEngine::SetCamera(Entity entity)
	{
		currentEngine->currentCameraEntity = entity;
		Registry& reg = currentEngine->registry;
		currentEngine->currentCamera = reg.GetComponent<Camera>(entity);
	}
	Entity GameEngine::GetCameraEntity()
	{
		return currentEngine->currentCameraEntity;
	}
	Camera GameEngine::GetCamera()
	{
		return currentEngine->currentCamera;
	}
	Registry& GetRegistry()
	{
		return currentEngine->registry;
	}
}


