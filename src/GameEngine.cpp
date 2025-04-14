#include "GameEngine.h"
#include <iostream>
#include <filesystem>
Engine::Engine()
{
}
void Engine::Start()
{
	std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
	defaultShader = ResourceManager::LoadShader("res/shaders/Basic.vert", "res/shaders/Basic.frag");
	ResourceManager::InitShaders();
	defaultMaterial = Material(*defaultShader.rendererID);
	Entity defaultEntity = ResourceManager::LoadEntityFromOBJ("res/models/cube.obj");
	Registry& reg = Registry::GetInstance();
	defaultMesh = reg.GetComponent<Mesh>(defaultEntity);
	reg.DestroyEntity(defaultEntity);
	printf("Game started");
}

void Engine::Run()
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

	void GameEngine::SetCurrentEngine(Engine* engine)
	{
		currentEngine = engine;
	}

	Entity GameEngine::CreateEmptyEntity()
	{
		Entity entity = Registry::GetInstance().CreateEntity();
		Registry& reg = Registry::GetInstance();
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
		Registry& registry = Registry::GetInstance();
		Entity entity = registry.CreateEntity();
		transform.position = glm::vec3(1.0f);
		registry.AssignComponent(entity, transform);
		registry.AssignComponent(entity, mesh);
		registry.AssignComponent(entity, material);
		return entity;
	}

	Entity GameEngine::CreateDirLight(const glm::vec3 direction, const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity)
	{
		Registry& registry = Registry::GetInstance();
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, DirectionalLight(direction, ambient, diffuse, specular, intensity));
		return entity;
	}

	Entity GameEngine::CreatePointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity, const float constant, const float linear, const float quadratic)
	{
		Registry& registry = Registry::GetInstance();
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, PointLight(ambient, diffuse, specular, intensity, constant, linear, quadratic));
		return entity;
	}
	void GameEngine::SetCamera(Entity entity)
	{
		currentEngine->currentCameraEntity = entity;
		Registry& reg = Registry::GetInstance();
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
}


