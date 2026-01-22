#include "engine.h"
#include <iostream>
#include <filesystem>
#include "platform/vulkan/vulkan.h"
#include "platform/opengl/opengl.h"
#include "time.h"
#include "platform/vulkan/vk_compute_manager.h"
#include "platform/opengl/gl_compute_manager.h"
#include <stdexcept>
Engine::Engine(const GraphicsAPIType graphicsAPIType)
{
	this->graphicsAPIType = graphicsAPIType;
	engine::SetInstance(this);

	resourceManager.CreateResourcePools();
	switch (graphicsAPIType)
	{
	case GRAPHICS_API_OPENGL:
		graphicsAPI = std::make_unique<OpenGl>();
		break;
	case GRAPHICS_API_VULKAN:
		graphicsAPI = std::make_unique<Vulkan>();
		break;
	default:
		std::cerr << "Unknown graphics API type!" << std::endl;
		exit(EXIT_FAILURE);
	}

}
void Engine::initialize(const std::shared_ptr<Window>& window)
{
	this->window = window; // Store the window pointer for later use
	if (graphicsAPI == nullptr)
	{
		std::cerr << "Graphics API not initialized!" << std::endl;
		exit(EXIT_FAILURE);
	}
	glfwSetWindowUserPointer(window->getHandle(), graphicsAPI.get());
	glfwSetFramebufferSizeCallback(window->getHandle(), graphicsAPI->frameBufferResizeCallback);
	graphicsAPI->initialize(window->getHandle()); // Pass the window if needed
	graphicsAPI->initializeImGui();
	this->computeManager = &graphicsAPI->computeManager;
	this->bufferManager = &graphicsAPI->bufferManager;

	std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
	ShaderCreateInfo create;
	create.alwaysRender = false;
	defaultShader = resourceManager.LoadShader(RES_DIR"shaders/Basic.vert", RES_DIR"shaders/Basic.frag", create);
	defaultMaterial = resourceManager.LoadMaterial(defaultShader);
	defaultMesh = resourceManager.LoadOBJ(RES_DIR"models/cube.obj");

	Entity camera = engine::CreateEmptyEntity();
	registry.AssignComponent<Camera>(camera, Camera());
	registry.AssignComponent<Transform>(camera, Transform());
	currentCameraEntity = camera;
	currentCamera = registry.GetComponent<Camera>(camera);
}
void Engine::Start()
{
	printf("Game started");
	registry.startSystems();
}

void Engine::Update()
{
	Time::Update();
	registry.updateSystems();
	renderer.DrawElements(registry, resourceManager, currentCameraEntity);
}

void Engine::LoadScene(const std::string& scenePath)
{
	// parse scene
	// load scene
}

void Engine::cleanup()
{
	resourceManager.unloadAll();
	graphicsAPI->cleanup();
}

Scene Engine::GetScene()
{
	return Scene();
}

Engine::~Engine()
{
}



namespace engine {
	namespace internal {
		Engine* _currentEnginePtr = nullptr;
	}

	void SetInstance(Engine* engineInstance)
	{
		internal::_currentEnginePtr = engineInstance;
	}

	Engine& engine::GetEngine()
	{
		return *internal::_currentEnginePtr;
	}

	Entity engine::CreateEmptyEntity()
	{
		Entity entity = GetEngine().registry.CreateEntity();
		Registry& reg = GetEngine().registry;
		return entity;
	}
	Entity engine::CreateDefaultObject(Transform transform)
	{
		return CreateObject(transform, GetEngine().defaultMesh, GetEngine().defaultMaterial);
	}
	Entity engine::CreateObject(Transform transform, Mesh mesh, Material material)
	{
		Registry& registry = GetEngine().registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, transform);
		registry.AssignComponent(entity, mesh);
		registry.AssignComponent(entity, material);
		return entity;
	}

	Entity engine::CreateDirLight(const glm::vec3 direction, const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity)
	{
		Registry& registry = GetEngine().registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, Transform());
		registry.AssignComponent(entity, DirectionalLight(direction, ambient, diffuse, specular, intensity));
		return entity;
	}

	Entity engine::CreatePointLight(const Transform transform, const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular, const float intensity, const float constant, const float linear, const float quadratic)
	{
		Registry& registry = GetEngine().registry;
		Entity entity = registry.CreateEntity();
		registry.AssignComponent(entity, transform);
		registry.AssignComponent(entity, PointLight(ambient, diffuse, specular, intensity, constant, linear, quadratic));
		return entity;
	}
	void engine::SetCamera(Entity entity)
	{
		GetEngine().currentCameraEntity = entity;
		Registry& reg = GetEngine().registry;
		GetEngine().currentCamera = reg.GetComponent<Camera>(entity);
	}
	const Entity engine::GetCameraEntity()
	{
		return GetEngine().currentCameraEntity;
	}
	Camera engine::GetCamera()
	{
		return GetEngine().currentCamera;
	}
	Registry& GetRegistry()
	{
		return GetEngine().registry;
	}
}
namespace graphics {
	std::unique_ptr<GraphicsAPI>& graphics::GetGraphicsAPI()
	{
		if (!engine::GetEngine().graphicsAPI) throw std::runtime_error("Graphics not initialized?");
		return engine::GetEngine().graphicsAPI;
	}
	GraphicsAPIType getGraphicsAPIType()
	{
		return engine::GetEngine().graphicsAPIType;
	}
}

