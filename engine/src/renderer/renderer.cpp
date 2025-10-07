#include "renderer/renderer.h"
#include "components/lights.h"
#include "components/camera.h"
#include "imgui/imgui.h"

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

	std::vector<MeshData*> meshes;
	std::vector<MaterialData*> materials;

	std::vector<Transform*>& transforms = currentView.GetComponents<Transform>();
	std::vector<glm::mat4x4> modelMatrices;
	modelMatrices.reserve(transforms.size());

	for (size_t i = 0; i < currentView.size(); i++)
	{
		meshes.emplace_back(&(*resourceManager.m_meshes)[std::get<2>(currentView[i])->id]);
		materials.emplace_back(&resourceManager.m_materials->at(std::get<3>(currentView[i])->handle));
		modelMatrices.emplace_back(registry.GetLocalToWorldMatrix(std::get<0>(currentView[i])) * transforms[i]->getMatrix());
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
