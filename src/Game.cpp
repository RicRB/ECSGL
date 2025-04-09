#include "Game.h"
#include <iostream>

Game::Game()
{
}
void Game::Start()
{
	defaultShader = ResourceManager::LoadShader("res/shaders/Basic.vert", "res/shaders/Basic.frag");
	ResourceManager::InitShaders();
	defaultMaterial = Material(*defaultShader.rendererID);
	Entity defaultEntity = ResourceManager::LoadEntityFromOBJ("res/models/cube.obj");
	Registry& reg = Registry::GetInstance();
	defaultMesh = reg.GetComponent<Mesh>(defaultEntity);
	reg.DestroyEntity(defaultEntity);
	printf("Game started");
}

void Game::Run()
{

}

void Game::LoadScene(const std::string& scenePath)
{
	// parse scene
	// load scene
}

Scene Game::GetScene()
{
	return Scene();
}







Entity GameEngine::CreateEmptyEntity(Transform transform)
{
	Entity entity = Registry::GetInstance().CreateEntity();
	Registry& reg = Registry::GetInstance();
	reg.AssignComponent(entity, transform);
	return entity;
}
Entity GameEngine::CreateObject(Transform transform, Mesh mesh, Material material)
{
	auto& game = Game::GetInstance();
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
	auto& game = Game::GetInstance();
	Registry& registry = Registry::GetInstance();
	Entity entity = registry.CreateEntity();
	registry.AssignComponent(entity, Transform());
	registry.AssignComponent(entity, DirectionalLight(direction, ambient, diffuse, specular, intensity));
	return entity;
}

Entity GameEngine::CreatePointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity, const float constant, const float linear, const float quadratic)
{
	auto& game = Game::GetInstance();
	Registry& registry = Registry::GetInstance();
	Entity entity = registry.CreateEntity();
	registry.AssignComponent(entity, Transform());
	registry.AssignComponent(entity, PointLight(ambient, diffuse, specular, intensity, constant, linear, quadratic));
	return entity;
}

void GameEngine::SetCamera(Entity entity)
{
	auto& game = Game::GetInstance();
	game.currentCameraEntity = entity;
	Registry& reg = Registry::GetInstance();
	game.currentCamera = reg.GetComponent<Camera>(entity);
}
Entity GameEngine::GetCameraEntity()
{
	return Game::GetInstance().currentCameraEntity;
}
Camera GameEngine::GetCamera()
{
	return Game::GetInstance().currentCamera;
}

