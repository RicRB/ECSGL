#include "ecs/ecs.hpp"
#include "core/engine.h"
#include "core/time.h"
#include "core/input.h"
class Raycaster : public System {
public:
	Material materials[2];
	Entity entity;
	Texture textures[2];
	int renderedFrame = 0;
	int resX = 500, rexY = 500;
	uint32_t groupCountX, groupCountY;
	std::unique_ptr<ComputeDispatcher> dispatcher;
	Raycaster() = default;
	void start() override {
		// Initialization code if needed
		auto camera = engine::GetCameraEntity();
		auto& registry = engine::GetRegistry();
		ShaderCreateInfo create;
		create.alwaysRender = true;
		Shader shader = engine::GetEngine().resourceManager.LoadShader(RES_DIR"shaders/simpleTexture.vert", RES_DIR"shaders/simpleTexture.frag", create);
		materials[0] = engine::GetEngine().resourceManager.LoadMaterial(shader);
		materials[1] = engine::GetEngine().resourceManager.LoadMaterial(shader);

		Mesh mesh = engine::GetEngine().resourceManager.LoadOBJ(RES_DIR"models/quad.obj");
		entity = engine::CreateObject(Transform(glm::vec3(), glm::vec3(0.0,180.0,0.0)), mesh, materials[renderedFrame]);

		textures[0] = engine::GetEngine().resourceManager.LoadEmptyTexture(resX, rexY, TextureUsage::ReadWrite);
		textures[1] = engine::GetEngine().resourceManager.LoadEmptyTexture(resX, rexY, TextureUsage::ReadWrite);

		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[0], textures[0], 0);
		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[1], textures[1], 0);

		ComputeShader cShader = engine::GetEngine().resourceManager.LoadComputeShader(RES_DIR"shaders/compute/test.glsl");
		dispatcher = std::make_unique<ComputeDispatcher>(cShader);

		groupCountX = (500 + 15) / 16;  // = 32 workgroups
		groupCountY = (500 + 15) / 16;  // = 32 workgroups

		dispatcher->setTexture("outputTexture", textures[1]);
		dispatcher->dispatch(groupCountX, groupCountY, 1);
	}
	void update() override {
		if (!dispatcher->isDone()) return;
		Transform transform = engine::GetRegistry().GetComponent<Transform>(entity);
		dispatcher->setTexture("outputTexture", textures[renderedFrame]);
		dispatcher->dispatch(groupCountX, groupCountY, 1);

		renderedFrame = (renderedFrame + 1) % 2;
		
		engine::GetRegistry().AssignComponent<Material>(entity, materials[renderedFrame]);

	}
	void cleanup() override {
		// Cleanup code if needed
	}
	const char* getName() const override {
		return "Raycaster";
	}
};