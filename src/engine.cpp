#include "engine.h"
#include <iostream>
#include <filesystem>
#include "renderer/vulkan.h"

Engine::Engine(const GraphicsAPIType graphicsAPIType)
{
	rendererType = graphicsAPIType;
	switch (graphicsAPIType)
	{
	case GRAPHICS_API_OPENGL:
		//graphicsAPI = std::make_unique<OpenGL>();
		break;
	case GRAPHICS_API_VULKAN:
		graphicsAPI = std::make_unique<Vulkan>();
		break;
	default:
		std::cerr << "Unknown graphics API type!" << std::endl;
		exit(EXIT_FAILURE);
	}
}
void Engine::initialize(GLFWwindow* window)
{
	engine::SetCurrentEngine(*this);
	this->window = window; // Store the window pointer for later use
	if (graphicsAPI == nullptr)
	{
		std::cerr << "Graphics API not initialized!" << std::endl;
		exit(EXIT_FAILURE);
	}
	graphicsAPI->initialize(window); // Pass the window if needed
}
void Engine::Start()
{
	std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
	defaultShader = resourceManager.LoadShader("res/shaders/Basic.vert", "res/shaders/Basic.frag");
	resourceManager.InitShaders(registry);
	defaultMesh = resourceManager.LoadOBJ("res/models/cube.obj");
	printf("Game started");
}

void Engine::Update()
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

Engine::~Engine()
{
	if (graphicsAPI)
	{
		graphicsAPI->cleanup();
		graphicsAPI.reset();
	}
}



namespace engine {
	Engine* currentEngine = nullptr;
	GraphicsAPIType* currentRendererType = nullptr;
	void engine::SetCurrentEngine(Engine& engine)
	{
		currentEngine = &engine;
		currentRendererType = &engine.rendererType;
	}

	Entity engine::CreateEmptyEntity()
	{
		Entity entity = currentEngine->registry.CreateEntity();
		Registry& reg = currentEngine->registry;
		return entity;
	}
	Entity engine::CreateObject(Transform transform, Mesh mesh, Material material)
	{
		Registry& registry = currentEngine->registry;
		Entity entity = registry.CreateEntity();
		transform.position = glm::vec3(1.0f);
		registry.AssignComponent(entity, transform);
		registry.AssignComponent(entity, mesh);
		registry.AssignComponent(entity, material);
		return entity;
	}

	Entity engine::CreateDirLight(const glm::vec3 direction, const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity)
	{
		Registry& registry = currentEngine->registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, DirectionalLight(direction, ambient, diffuse, specular, intensity));
		return entity;
	}

	Entity engine::CreatePointLight(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity, const float constant, const float linear, const float quadratic)
	{
		Registry& registry = currentEngine->registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, PointLight(ambient, diffuse, specular, intensity, constant, linear, quadratic));
		return entity;
	}
	void engine::SetCamera(Entity entity)
	{
		currentEngine->currentCameraEntity = entity;
		Registry& reg = currentEngine->registry;
		currentEngine->currentCamera = reg.GetComponent<Camera>(entity);
	}
	Entity engine::GetCameraEntity()
	{
		return currentEngine->currentCameraEntity;
	}
	Camera engine::GetCamera()
	{
		return currentEngine->currentCamera;
	}
	Registry& GetRegistry()
	{
		return currentEngine->registry;
	}
}


