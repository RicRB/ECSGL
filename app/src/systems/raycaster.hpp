#include "ecs/ecs.hpp"
#include "core/engine.h"
#include "core/time.h"
#include "core/input.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
	Buffer objectBuffer;
	double mass;
	double radius;
	double r_s;
	const double c = 299792458.0;
	const double G = 6.67430e-11;


	struct RotationCamera {
		float radius;
		float minRadius = 1e10f, maxRadius = 1e12f;

		float azimuth = 0.0f;
		float elevation = M_PI / 2.0f;

		float orbitSpeed = 0.1f;
		float panSpeed = 0.01f;
		double zoomSpeed = 25e9f;

		// Calculate camera position in world space
		glm::vec3 position() const {
			float clampedElevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
			// Orbit around (0,0,0) always
			return glm::vec3(
				radius * sin(clampedElevation) * cos(azimuth),
				radius * cos(clampedElevation),
				radius * sin(clampedElevation) * sin(azimuth)
			);
		}

	};

	RotationCamera rotationCamera;


	struct Object {
		glm::vec3 position;
		float radius;
	};

	struct ObjectData {
		int count;
		Object objects[1];
	};

	ObjectData objectData;

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

		objectData.objects[0].position = glm::vec3(-r_s * 2, 0.0, 0.0);
		objectData.objects[0].radius = r_s*3;
		objectData.count = 1;
		{
			BufferDescription desc;
			desc.type = BufferType::SSBO;
			desc.size = sizeof(ObjectData);
			objectBuffer = engine::GetBufferManager().createBuffer(desc);
		}
		engine::GetBufferManager().updateBuffer(objectBuffer, &objectData, sizeof(ObjectData));

		rotationCamera.radius = r_s * 8;
		auto& camTransform = registry.GetComponent<Transform>(camera);
		camTransform.setCaching(false);

		updateData();


		ComputeShader cShader = engine::GetEngine().resourceManager.LoadComputeShader(RES_DIR"shaders/compute/test.glsl");
		dispatcher = std::make_unique<ComputeDispatcher>(cShader);

		groupCountX = (resX + 7) / 8;  // = 32 workgroups
		groupCountY = (resY + 7) / 8;  // = 32 workgroups

		dispatcher->setTexture("outputImage", textures[1]);
		dispatcher->setBuffer("inputData", buffer);
		dispatcher->setBuffer("objectData", objectBuffer);
		dispatcher->dispatch(groupCountX, groupCountY, 1);
	}
	void update() override {

		if (!dispatcher->isDone()) return;
		updateData();


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

	void updateData()
	{
		HandleInputs();
		auto camera = engine::GetCameraEntity();
		auto& registry = engine::GetRegistry();
		auto& camTransform = registry.GetComponent<Transform>(camera);


		camTransform.setPosition(rotationCamera.position());
		glm::vec3 fwd = glm::normalize(-camTransform.getPosition());
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
		data.r1 = r_s * 1.5f;
		data.r2 = r_s * 3.0f;
	}

	void HandleInputs()
	{
		if (Input::GetKeyPressed(Key::W)) {
			rotationCamera.elevation -= rotationCamera.orbitSpeed * Time::getDeltaTime();
		}
		if (Input::GetKeyPressed(Key::S)) {
			rotationCamera.elevation += rotationCamera.orbitSpeed * Time::getDeltaTime();
		}
		if (Input::GetKeyPressed(Key::A)) {
			rotationCamera.azimuth += rotationCamera.orbitSpeed * Time::getDeltaTime();
		}
		if (Input::GetKeyPressed(Key::D)) {
			rotationCamera.azimuth -= rotationCamera.orbitSpeed * Time::getDeltaTime();
		}
		double yScroll = Input::GetVerticalMouseScroll();
		rotationCamera.radius -= yScroll * rotationCamera.zoomSpeed;
	}
};