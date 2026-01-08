#include "renderer/renderer.h"
#include "components/lights.h"
#include "components/camera.h"
#include "imgui/imgui.h"
#include <set>
#include <array>
Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Run()
{
}
void Renderer::DrawElements(Registry& registry, ResourceManager& resourceManager, Entity camera)
{
	auto currentView = registry.GetView<Transform, Mesh, Material>();
	if (graphicsAPI == nullptr) {
		return;
	}

	Transform& cameraTransform = registry.GetComponent<Transform>(camera);
	cameraTransform.setCaching(false);
	Camera& cameraComponent = registry.GetComponent<Camera>(camera);

	const size_t entityCount = currentView.size();
	std::vector<MeshData*> meshes;
	meshes.reserve(entityCount);
	std::vector<MaterialData*> materials;
	materials.reserve(entityCount);

	std::vector<Transform*>& transforms = currentView.GetComponents<Transform>();

	std::vector<glm::mat4x4> modelMatrices;
	modelMatrices.reserve(entityCount);
	
	std::set<ShaderDataHandle> usedShaders;
	std::unordered_map<ShaderDataHandle, std::vector<size_t>> shaderEntities;
	for (size_t i = 0; i < currentView.size(); i++)
	{
		auto material = currentView.GetComponent<Material>(i);
		// view.GetComponent<Material>
		// SORTING BY SHADER TO MINIMIZE STATE CHANGES
		auto shaderHandle = (*resourceManager.m_materials)[material->handle].shaderHandle;
		if (usedShaders.find(shaderHandle) == usedShaders.end())
		{
			usedShaders.insert(shaderHandle);
			shaderEntities[shaderHandle] = std::vector<size_t>();
		}
		shaderEntities[shaderHandle].push_back(i);
		meshes.emplace_back(&(*resourceManager.m_meshes)[currentView.GetComponent<Mesh>(i)->handle]);
		materials.emplace_back(&resourceManager.m_materials->at(currentView.GetComponent<Material>(i)->handle));
		modelMatrices.emplace_back(registry.GetLocalToWorldMatrix(currentView.GetEntity(i)) * transforms[i]->getMatrix());
	}

	// GLOBAL UNIFORM OBJECT

	GlobalUBO globalUniformObject;
	globalUniformObject.dirLightCount = 0;
	globalUniformObject.pointLightCount = 0;

	if (registry.PoolExists<DirectionalLight>()) {
		auto dirView = registry.GetView<DirectionalLight>();
		auto dirLightsView = dirView.GetComponents<DirectionalLight>();
		for (size_t i = 0; i < dirView.size(); i++)
		{
			auto dirLight = dirLightsView[i];
			DirectionLightUniform& ubo = globalUniformObject.dirLights[i];
			ubo.direction = glm::vec4(dirLight->direction, 1.0);
			ubo.ambient = glm::vec4(dirLight->ambient, 1.0);
			ubo.diffuse = glm::vec4(dirLight->diffuse, 1.0);
			ubo.specular = glm::vec4(dirLight->specular, 1.0);

			globalUniformObject.dirLights[globalUniformObject.dirLightCount] = ubo;
		}
		globalUniformObject.pointLightCount = static_cast<uint32_t>(dirView.size());
	}

	if (registry.PoolExists<PointLight>()) {
		auto pointView = registry.GetView<PointLight, Transform>();
		auto pointLightsView = pointView.GetComponents<PointLight>();
		auto pointTransforms = pointView.GetComponents<Transform>();
		for (size_t i = 0; i < pointView.size(); i++)
		{
			PointLightUniform ubo;
			glm::mat4 localToWorld = registry.GetLocalToWorldMatrix(std::get<0>(pointView[i]));
			ubo.position = localToWorld * glm::vec4(pointTransforms[i]->getPosition(), 1.0);
			ubo.constant = pointLightsView[i]->constant;
			ubo.linear = pointLightsView[i]->linear;
			ubo.quadratic = pointLightsView[i]->quadratic;
			ubo.ambient = glm::vec4(pointLightsView[i]->ambient, 1.0);
			ubo.diffuse = glm::vec4(pointLightsView[i]->diffuse, 1.0);
			ubo.specular = glm::vec4(pointLightsView[i]->specular, 1.0);

			globalUniformObject.pointLights[i] = ubo;
		}
		globalUniformObject.pointLightCount = static_cast<uint32_t>(pointView.size());
	}
	// ImGui
	times += 1.0f;
	if (times >= 1000)
	{
		times = 0;
		fps = 0.0f;
	}
	if (graphicsAPI->isImGuiInitialized)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		fps = fps + (1000.0f / io.Framerate);
		graphicsAPI->beginDrawImGui();
		ImGui::Begin("Hello, world!");
		ImGui::DragFloat3("Camera pos ", &cameraTransform.getPositionReference().x);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::Text("Average Frame time : %.3f ms", fps / times);
		ImGui::End();
		graphicsAPI->endDrawImGui();
	}
	FrameDrawData frameDrawData{ cameraTransform.viewMatrix(),
		cameraComponent.projection,
		&globalUniformObject,
		currentView.size(),
		modelMatrices,
		meshes,
		materials,
		*resourceManager.m_shaders
	};
	graphicsAPI->drawFrame(frameDrawData);
}

void Renderer::Clear() const
{
}
