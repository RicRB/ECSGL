#include "ecs/ecs.hpp"
#include "core/engine.h"
#include "core/time.h"
#include "core/input.h"
class Raycaster : public System {
public:
	Material material;
	Entity entity;
	Texture textures[2];
	int currentText = 0;
	int resX = 500, rexY = 500;
	uint32_t groupCountX, groupCountY;
	std::unique_ptr<ComputeDispatcher> dispatcher;
	Raycaster() = default;
	void start() override {
		// Initialization code if needed
		auto camera = engine::GetCameraEntity();
		auto& registry = engine::GetRegistry();
		Shader shader = engine::GetEngine().resourceManager.LoadShader(RES_DIR"shaders/simpleTexture.vert", RES_DIR"shaders/simpleTexture.frag");
		material = engine::GetEngine().resourceManager.LoadMaterial(shader);

		Mesh mesh = engine::GetEngine().resourceManager.LoadOBJ(RES_DIR"models/quad.obj");
		entity = engine::CreateObject(Transform(), mesh, material);

		textures[0] = engine::GetEngine().resourceManager.LoadEmptyTexture(resX, rexY, TextureUsage::ReadWrite);
		textures[1] = engine::GetEngine().resourceManager.LoadEmptyTexture(resX, rexY, TextureUsage::ReadWrite);

		ComputeShader cShader = engine::GetEngine().resourceManager.LoadComputeShader(RES_DIR"shaders/compute/test.glsl");
		dispatcher = std::make_unique<ComputeDispatcher>(cShader);

		groupCountX = (500 + 15) / 16;  // = 32 workgroups
		groupCountY = (500 + 15) / 16;  // = 32 workgroups

		dispatcher->setTexture("outputTexture", textures[currentText]);
		dispatcher->dispatch(groupCountX, groupCountY, 1);
	}
	void update() override {
		if (!dispatcher->isDone()) return;

		engine::GetEngine().resourceManager.bindTextureToMaterial(material, textures[currentText], 0);

		currentText = (currentText + 1) % 2;
		
		dispatcher->setTexture("outputTexture", textures[currentText]);

		dispatcher->dispatch(groupCountX, groupCountY, 1);
	}
	void cleanup() override {
		// Cleanup code if needed
	}
	const char* getName() const override {
		return "Raycaster";
	}
};