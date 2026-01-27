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
	int resX = 500, resY = 500;
	uint32_t groupCountX, groupCountY;
	std::unique_ptr<ComputeDispatcher> dispatcher;

	struct BlackHoleData {
		glm::vec3 pos; float _pad0;
		glm::vec3 right; float _pad1;
		glm::vec3 up; float _pad2;
		glm::vec3 forward; float _pad3;
		float tanHalfFov;
		float aspect;
		int width;
		int height;
		float r1;
		float r2;
	};

	BlackHoleData data;
	Buffer buffer;
	double mass;
	double radius;
	double r_s;
	const double c = 299792458.0;
	const double G = 6.67430e-11;
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

		textures[0] = engine::GetEngine().resourceManager.LoadEmptyTexture(resX, resY, TextureUsage::ReadWrite);
		textures[1] = engine::GetEngine().resourceManager.LoadEmptyTexture(resX, resY, TextureUsage::ReadWrite);

		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[0], textures[0], 0);
		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[1], textures[1], 0);

		// buffer
		{
			BufferDescription desc;
			desc.type = BufferType::SSBO;
			desc.size = sizeof(BlackHoleData);
			buffer = engine::GetBufferManager().createBuffer(desc);
		}


		mass = 8.54e36;
		r_s = 2.0 * G * mass / (c * c);


		


		ComputeShader cShader = engine::GetEngine().resourceManager.LoadComputeShader(RES_DIR"shaders/compute/test.glsl");
		dispatcher = std::make_unique<ComputeDispatcher>(cShader);

		groupCountX = (500 + 15) / 16;  // = 32 workgroups
		groupCountY = (500 + 15) / 16;  // = 32 workgroups

		dispatcher->setTexture("outputImage", textures[1]);
		dispatcher->dispatch(groupCountX, groupCountY, 1);
	}
	void update() override {
		if (!dispatcher->isDone()) return;

		auto camera = engine::GetCameraEntity();
		auto& registry = engine::GetRegistry();
		auto& camTransform = registry.GetComponent<Transform>(camera);

		camTransform.setCaching(false);

		camTransform.setPosition(glm::vec3(r_s*8, -r_s, 0.0));
		glm::vec3 fwd = glm::normalize(camTransform.getForward());
		glm::vec3 up = glm::vec3(0, 1, 0); // y axis is up, so disk is in x-z plane
		glm::vec3 right = glm::normalize(glm::cross(fwd, up));
		up = glm::cross(right, fwd);

		data.pos = camTransform.getPosition();
		data.right = right;
		data.up = up;
		data.forward = fwd;
		data.tanHalfFov = glm::tan(glm::radians(60.0f * 0.5f));
		data.aspect = float(resX) / float(resY);
		data.width = resX;
		data.height = resY;
		data.r1 = r_s * 2.2f;
		data.r2 = r_s * 5.2f;


		engine::GetBufferManager().updateBuffer(buffer, &data,  sizeof(BlackHoleData));

		Transform transform = engine::GetRegistry().GetComponent<Transform>(entity);
		dispatcher->setTexture("outputImage", textures[renderedFrame]);
		dispatcher->setBuffer("inputData", buffer);
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