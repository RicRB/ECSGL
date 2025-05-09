#pragma once
#include "ECS/ECS.hpp"
#include "Shader.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "ResourceManager.h"
#include "Lights.h"
#include "Camera.h"
class Engine
{
public:
	//std::unique_ptr<Scene> currentScene;
	Registry registry;
	ResourceManager resourceManager;
	Shader defaultShader;
	Material defaultMaterial;
	Mesh defaultMesh;
	Camera currentCamera;
	Entity currentCameraEntity;
	
public:
	Engine();
	void Start();
	void Update();
	void LoadScene(const std::string& scenePath);
	void SetCamera(Entity camera);
	Scene GetScene();

private:

	Engine(const Engine&) = delete;
};
// do an if guard

namespace GameEngine
{
	extern Engine* currentEngine;
	void SetCurrentEngine(Engine& engine);
	Entity CreateEmptyEntity();
	Entity CreateObject(Transform transform = Transform(), Mesh mesh = {}, Material material = {});
	Entity CreateDirLight(const glm::vec3 direction = glm::vec3(0.0f), const glm::vec3 ambient = glm::vec3(0.0f), const glm::vec3 diffuse = glm::vec3(1.0f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 2.0f);
	Entity CreatePointLight(const glm::vec3 ambient = glm::vec3(0.5f), const glm::vec3 diffuse = glm::vec3(0.2f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 1.0f, const float constant = 1.0f, const float linear = 0.09f, const float quadratic = 0.032f);
	void SetCamera(Entity entity);
	Entity GetCameraEntity();
	Camera GetCamera();
	Registry& GetRegistry();
};