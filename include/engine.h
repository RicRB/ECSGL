#pragma once
#include "ecs/ecs.hpp"
#include "shader.h"
#include "transform.h"
#include "mesh.h"
#include "material.h"
#include "resource_manager.h"
#include "lights.h"
#include "camera.h"
#include "renderer.h"
#include <memory>

class Engine
{
public:
	//std::unique_ptr<Scene> currentScene;
	Registry registry;
	ResourceManager resourceManager;
	Renderer renderer;
	Shader defaultShader;
	Material defaultMaterial;
	Mesh defaultMesh;
	Camera currentCamera;
	Entity currentCameraEntity;
	GLFWwindow* window;
	
public:
	Engine(const GraphicsAPIType graphicsAPIType);
	void initialize(GLFWwindow* window);
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
	extern Engine* currentEngine;
	extern GraphicsAPIType* currentRendererType;
	void SetCurrentEngine(Engine& engine);
	Entity CreateEmptyEntity();
	Entity CreateDefaultObject(Transform transform = Transform());
	Entity CreateObject(Transform transform = Transform(), Mesh mesh = {}, Material material = {});
	Entity CreateDirLight(const glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f), const glm::vec3 ambient = glm::vec3(0.0f), const glm::vec3 diffuse = glm::vec3(1.0f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 2.0f);
	Entity CreatePointLight(const glm::vec3 ambient = glm::vec3(0.0f), const glm::vec3 diffuse = glm::vec3(0.2f), const glm::vec3 specular = glm::vec3(0.2f), const float intensity = 1.0f, const float constant = 1.0f, const float linear = 0.09f, const float quadratic = 0.032f);
	void SetCamera(Entity entity);
	Entity GetCameraEntity();
	Camera GetCamera();
	Registry& GetRegistry();
};