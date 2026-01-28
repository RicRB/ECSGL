#include "ecs/ecs.hpp"
#include "core/engine.h"
#include "core/time.h"
#include "core/input.h"
#include "imgui/imgui.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Raycaster : public System {
public:
	Material materials[2];
	Entity entity;
	Texture textures[2];
	Texture sphereMap;
	int renderedFrame = 0;
	int resX = 1000, resY = 1000;
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
		int maxSteps;
		float stepSizeMin;
		float stepSizeMax;
		float r_s;
		float escape_r;
		float rotSpeed;
		float mass;
	};

	BlackHoleData data;
	Buffer buffer;
	Buffer objectBuffer;


	struct RotationCamera {
		float radius;
		float minRadius = 10, maxRadius = 100;

		float azimuth = 0.0f;
		float elevation = M_PI / 2.0f;

		float orbitSpeed = 1.0f;
		float panSpeed = 1.0f;
		double zoomSpeed = 5;

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


	struct alignas(16) Object {
		glm::vec3 position;
		float radius;
		glm::vec4 color;
		Object() = default;
		Object(glm::vec3 position, float radius, glm::vec4 color) : position(position), radius(radius), color(color) {};
	};

	struct alignas(16) ObjectData {
		int count;
		Object objects[3];
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
		sphereMap = engine::GetEngine().resourceManager.LoadTexture(RES_DIR"images/background.hdr", TextureUsage::Read);


		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[0], textures[0], 0);
		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[1], textures[1], 0);


		auto& camTransform = registry.GetComponent<Transform>(camera);
		camTransform.setCaching(false);
		data.r_s = 5.0f;
		data.width = resX;
		data.height = resY;
		data.r1 = data.r_s * 1.5f;
		data.r2 = data.r_s * 3.0f;
		data.maxSteps = 1000;
		data.stepSizeMin = 0.1f;
		data.stepSizeMax = 0.3f;

		data.escape_r = data.r_s * 100;
		data.rotSpeed = 0.6f;
		data.mass = 2.4f;


		// buffer
		{
			BufferDescription desc;
			desc.type = BufferType::SSBO;
			desc.size = sizeof(BlackHoleData);
			buffer = engine::GetBufferManager().createBuffer(desc);
		}

		rotationCamera.radius = data.r_s * 8;
		objectData.objects[0] = Object(-rotationCamera.position() * 2.0f, data.r_s * 1.0f, glm::vec4(0.0, 1.0, 0.0, 1.0));
		objectData.objects[1] = Object(glm::vec3(-rotationCamera.position().x * 3.0f, 0, 0), data.r_s * 1.3f, glm::vec4(1.0, 0.0, 0.0, 1.0));
		objectData.objects[2] = Object(glm::vec3(-rotationCamera.position().x * 2.0f, 0, -rotationCamera.position().x * 2.0f), data.r_s * 0.6f, glm::vec4(0.0, 0.0, 1.0, 1.0));
		objectData.count = 3;

		{
			BufferDescription desc;
			desc.type = BufferType::SSBO;
			desc.size = sizeof(ObjectData);
			objectBuffer = engine::GetBufferManager().createBuffer(desc);
		}
		engine::GetBufferManager().updateBuffer(objectBuffer, &objectData, sizeof(ObjectData));



		updateData();


		ComputeShader cShader = engine::GetEngine().resourceManager.LoadComputeShader(RES_DIR"shaders/compute/raymarcher.glsl");
		dispatcher = std::make_unique<ComputeDispatcher>(cShader);

		groupCountX = (resX + 7) / 8;  // = 32 workgroups
		groupCountY = (resY + 7) / 8;  // = 32 workgroups

		dispatcher->setTexture("outputImage", textures[1]);
		dispatcher->setBuffer("inputData", buffer);
		dispatcher->setBuffer("objectData", objectBuffer);
		dispatcher->setTexture("sphereMapTexture", sphereMap);
		dispatcher->dispatch(groupCountX, groupCountY, 1);
	}
	void update() override {
		HandleInputs();
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

	void onGui() override {
		ImGui::Begin("BLACK HOLE");
		ImGui::DragFloat("Radius", &data.r_s);
		ImGui::DragFloat("stepSizeMin", &data.stepSizeMin);
		ImGui::DragFloat("stepSizeMax", &data.stepSizeMax);
		ImGui::DragInt("maxSteps", &data.maxSteps);
		ImGui::DragFloat("Speed", &data.rotSpeed);
		ImGui::DragFloat("Mass", &data.mass);
		ImGui::End();
	}

	void cleanup() override {
		// Cleanup code if needed
	}
	const char* getName() const override {
		return "Raycaster";
	}

	void updateData()
	{
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