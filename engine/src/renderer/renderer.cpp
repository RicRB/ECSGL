#include "renderer/renderer.h"
#include "components/lights.h"
#include "components/camera.h"
#include "imgui/imgui.h"
#include <array>
#include <algorithm>
#include "core/engine.h"
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
	if (graphics::GetGraphicsAPI() == nullptr) {
		return;
	}
	auto& graphicsAPI = graphics::GetGraphicsAPI();
	Transform& cameraTransform = registry.GetComponent<Transform>(camera);
	cameraTransform.setCaching(false);
	Camera& cameraComponent = registry.GetComponent<Camera>(camera);

	meshes.clear();
	materials.clear();
	modelMatrices.clear();

	// get our normal objects
	currentView.each([&](Entity entity, Transform& transform, Mesh& mesh, Material& material) {
		meshes.emplace_back(&(*resourceManager.m_meshes)[mesh.handle]);
		materials.emplace_back(&resourceManager.m_materials->at(material.handle));
		modelMatrices.emplace_back(registry.GetLocalToWorldMatrix(entity) * transform.getMatrix());
	});
	std::reverse(meshes.begin(), meshes.end());
	std::reverse(materials.begin(), materials.end());
	std::reverse(modelMatrices.begin(), modelMatrices.end());

	GlobalUBO globalUniformObject;
	globalUniformObject.dirLightCount = 0;
	globalUniformObject.pointLightCount = 0;

	constexpr uint32_t maxDirLights = sizeof(GlobalUBO::dirLights) / sizeof(DirectionLightUniform);
	constexpr uint32_t maxPointLights = sizeof(GlobalUBO::pointLights) / sizeof(PointLightUniform);

	// get our lights
	if (registry.PoolExists<DirectionalLight>()) {
		registry.GetView<DirectionalLight>().each([&](Entity entity, DirectionalLight& dirLight) {
			if (globalUniformObject.dirLightCount >= maxDirLights) return;
			DirectionLightUniform& ubo = globalUniformObject.dirLights[globalUniformObject.dirLightCount++];
			ubo.direction = glm::vec4(dirLight.direction, 1.0);
			ubo.ambient = glm::vec4(dirLight.ambient, 1.0);
			ubo.diffuse = glm::vec4(dirLight.diffuse, 1.0);
			ubo.specular = glm::vec4(dirLight.specular, 1.0);
		});
	}

	// get our point lights
	if (registry.PoolExists<PointLight>()) {
		registry.GetView<PointLight, Transform>().each([&](Entity entity, PointLight& pointLight, Transform& transform) {
			if (globalUniformObject.pointLightCount >= maxPointLights) return;
			PointLightUniform& ubo = globalUniformObject.pointLights[globalUniformObject.pointLightCount++];
			glm::mat4 localToWorld = registry.GetLocalToWorldMatrix(entity);
			ubo.position = localToWorld * glm::vec4(transform.getPosition(), 1.0);
			ubo.intensity = pointLight.intensity;
			ubo.constant = pointLight.constant;
			ubo.linear = pointLight.linear;
			ubo.quadratic = pointLight.quadratic;
			ubo.ambient = glm::vec4(pointLight.ambient, 1.0);
			ubo.diffuse = glm::vec4(pointLight.diffuse, 1.0);
			ubo.specular = glm::vec4(pointLight.specular, 1.0);
		});
	}

	FrameDrawData frameDrawData{ cameraTransform.viewMatrix(),
		cameraComponent.projection,
		&globalUniformObject,
		static_cast<int>(modelMatrices.size()),
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
