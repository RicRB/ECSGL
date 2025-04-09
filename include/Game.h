#pragma once
#include "ECS/ECS.hpp"
#include "Shader.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "ResourceManager.h"
#include "Lights.h"
#include "Camera.h"
class Game
{
public:
	//std::unique_ptr<Scene> currentScene;
	Shader defaultShader;
	Material defaultMaterial;
	Mesh defaultMesh;
	Camera currentCamera;
	Entity currentCameraEntity;
	
public:
	void Start();
	void Run();
	void LoadScene(const std::string& scenePath);
	Scene GetScene();

	Game(const Game&) = delete;
	static Game& GetInstance()
	{
		static Game s_Instance;
		return s_Instance;
	}

private:
	Game();
};
// do an if guard

namespace GameEngine
{
	/*Entity GetCurrentCamera()
	{
		return Game::GetInstance().currentScene->currentCamera;
	}*/
	//void LoadScene(std::unique_ptr<Scene>&scene)
	//{
		//Game::GetInstance().LoadScene(scene);
	//}

	// DO THIS OBJECT SHIT IDK HOW TF IM GONNA DO IT I NEED DEFAULT SETTINGS THOUGH
	// object = Entity that is rendered
	Entity CreateEmptyEntity(Transform transform = Transform());
	Entity CreateObject(Transform transform = Transform(), Mesh mesh = Game::GetInstance().defaultMesh, Material material = Game::GetInstance().defaultMaterial);
	Entity CreateDirLight(const glm::vec3 direction = glm::vec3(0.0f), const glm::vec3 ambient = glm::vec3(0.0f), const glm::vec3 diffuse = glm::vec3(1.0f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 2.0f);
	Entity CreatePointLight(const glm::vec3 ambient = glm::vec3(0.5f), const glm::vec3 diffuse = glm::vec3(0.2f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 1.0f, const float constant = 1.0f, const float linear = 0.09f, const float quadratic = 0.032f);
	void SetCamera(Entity entity);
	Entity GetCameraEntity();
	Camera GetCamera();
};