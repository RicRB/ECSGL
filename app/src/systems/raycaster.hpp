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
	Texture noiseTexture;
	Texture directionTexture;
	Texture volumeTexture;
	int renderedFrame = 0;
	int lowResX = 1280/2, lowResY = 720/2;
	int fullResX = 1280, fullResY = 720;
	uint32_t groupCountX, groupCountY;
	uint32_t renderGroupCountX, renderGroupCountY;
	std::unique_ptr<ComputeDispatcher> dispatcher;
	std::unique_ptr<ComputeDispatcher> renderDispatcher;

	struct BlackHoleData {
		glm::vec3 pos; float _pad0;
		glm::vec3 right; float _pad1;
		glm::vec3 up; float _pad2;
		glm::vec3 forward; float _pad3;
		float tanHalfFov;
		float aspect;
		int width;
		int height;
		int maxSteps = 500;
		float stepSizeMin = 0.1f;
		float stepSizeMax = 5.0f;
		float escape_r = 500;
		float rotSpeed = 0.5f;
		float mass = 1.0f;
		
		int diskEnable = 1;
		float r1 = 3.5f;
		float r2 = 10.0f;
		float diskH = 0.2f;
		float diskEdgeK = 0.3f;

		float diskDensity = 3.0f;

		float sigmaT = 2.0f;
		float dsVolScale = 1.0f;
		int volSubsteps = 40;


		float time = 0.0f;
		float sampleScale = 1.0f;
		float spin = 20.0f;
		float gravAcceleration = 1.0f;
		float bhRotation = 0.0f;
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


	struct NoiseParams {
		int size;
		float scale;
		float boost;
		int octaves;
		int tileSize;
		float lacunarity;
		float gain;
		float time;
	};

	NoiseParams noiseParams;
	Buffer noiseParamsBuffer;
	std::unique_ptr<ComputeDispatcher> noiseDispatcher;

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

		// TEXTURES
		{
			TextureCreateInfo lowResInfo;
			lowResInfo.width = lowResX;
			lowResInfo.height = lowResY;
			lowResInfo.depth = 1;
			lowResInfo.usage = TextureUsage::ReadWrite;
			lowResInfo.format = TextureFormat::R16G16_SFLOAT;
			directionTexture = engine::GetEngine().resourceManager.LoadEmptyTexture(lowResInfo);
			TextureCreateInfo lowResInfo2 = lowResInfo;
			lowResInfo2.format = TextureFormat::R16G16B16A16_UNORM;
			volumeTexture = engine::GetEngine().resourceManager.LoadEmptyTexture(lowResInfo2);
		}


		

		{
			TextureCreateInfo fullResInfo;
			fullResInfo.width = fullResX;
			fullResInfo.height = fullResY;
			fullResInfo.depth = 1;
			fullResInfo.usage = TextureUsage::ReadWrite;
			fullResInfo.format = TextureFormat::R8G8B8A8_UNORM;
			textures[0] = engine::GetResourceManager().LoadEmptyTexture(fullResInfo);
			textures[1] = engine::GetResourceManager().LoadEmptyTexture(fullResInfo);
		}

		sphereMap = engine::GetEngine().resourceManager.LoadTexture(RES_DIR"images/background.hdr", TextureUsage::Read);

		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[0], textures[0], 0);
		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[1], textures[1], 0);


		auto& camTransform = registry.GetComponent<Transform>(camera);
		camTransform.setCaching(false);
		data.width = lowResX;
		data.height = lowResY;

		// buffer
		{
			BufferDescription desc;
			desc.type = BufferType::SSBO;
			desc.size = sizeof(BlackHoleData);
			buffer = engine::GetBufferManager().createBuffer(desc);
		}
		float horizonRadius = data.mass + glm::sqrt(data.mass * data.mass - data.rotSpeed * data.rotSpeed);
		rotationCamera.radius = horizonRadius*8;
		objectData.objects[0] = Object(-rotationCamera.position() * 2.0f, horizonRadius * 1.0f, glm::vec4(0.0, 1.0, 0.0, 1.0));
		objectData.objects[1] = Object(glm::vec3(-rotationCamera.position().x * 3.0f, 0, 0), horizonRadius * 1.3f, glm::vec4(1.0, 0.0, 0.0, 1.0));
		objectData.objects[2] = Object(glm::vec3(-rotationCamera.position().x * 2.0f, 0, -rotationCamera.position().x * 2.0f), horizonRadius * 0.6f, glm::vec4(0.0, 0.0, 1.0, 1.0));
		objectData.count = 3;

		{
			BufferDescription desc;
			desc.type = BufferType::SSBO;
			desc.size = sizeof(ObjectData);
			objectBuffer = engine::GetBufferManager().createBuffer(desc);
		}
		engine::GetBufferManager().updateBuffer(objectBuffer, &objectData, sizeof(ObjectData));



		updateData();

		generateNoiseMap();

		ComputeShader cShader = engine::GetEngine().resourceManager.LoadComputeShader(RES_DIR"shaders/compute/raymarcher2.glsl");
		dispatcher = std::make_unique<ComputeDispatcher>(cShader);

		groupCountX = (lowResX + 7) / 8;  // = 32 workgroups
		groupCountY = (lowResY + 7) / 8;  // = 32 workgroups

		dispatcher->setBuffer("inputData", buffer);
		dispatcher->setBuffer("objectData", objectBuffer);
		dispatcher->setTexture("noiseMap", noiseTexture);
		dispatcher->setTexture("directionTexture", directionTexture);
		dispatcher->setTexture("volumeTexture", volumeTexture);
		dispatcher->dispatch(groupCountX, groupCountY, 1);


		ComputeShader renderShader = engine::GetResourceManager().LoadComputeShader(RES_DIR"shaders/compute/blackHoleRender.glsl");
		renderDispatcher = std::make_unique<ComputeDispatcher>(renderShader);

		renderGroupCountX = (fullResX + 7) / 8;  // = 32 workgroups
		renderGroupCountY = (fullResY + 7) / 8;  // = 32 workgroups
		

		dispatcher->wait();

		renderDispatcher->setTexture("directionTexture", directionTexture);
		renderDispatcher->setTexture("volumeTexture", volumeTexture);
		renderDispatcher->setTexture("sphereMapTexture", sphereMap);
		renderDispatcher->setTexture("outputImage", textures[1]);
		renderDispatcher->dispatch(renderGroupCountX, renderGroupCountY, 1);
	}
	void update() override {
		HandleInputs();
		if (!renderDispatcher->isDone()) return;
		updateData();


		engine::GetBufferManager().updateBuffer(buffer, &data,  sizeof(BlackHoleData));

		Transform transform = engine::GetRegistry().GetComponent<Transform>(entity);
		//dispatcher->setBuffer("inputData", buffer);

		dispatcher->dispatchAndWait(groupCountX, groupCountY, 1);

		renderDispatcher->setTexture("outputImage", textures[renderedFrame]);
		renderDispatcher->dispatch(renderGroupCountX, renderGroupCountY, 1);
		renderedFrame = (renderedFrame + 1) % 2;
		
		engine::GetRegistry().AssignComponent<Material>(entity, materials[renderedFrame]);

	}

	void onGui() override {
		ImGui::Begin("BLACK HOLE");
		ImGui::DragFloat("stepSizeMin", &data.stepSizeMin);
		ImGui::DragFloat("stepSizeMax", &data.stepSizeMax);
		ImGui::DragInt("maxSteps", &data.maxSteps);
		ImGui::DragFloat("Rotation Speed", &data.rotSpeed, 0.1, 0, 5);
		ImGui::DragFloat("Mass", &data.mass);
		ImGui::DragFloat("InnerRadius", &data.r1);
		ImGui::DragFloat("OuterRadius", &data.r2);
		ImGui::DragFloat("Density", &data.diskDensity);
		ImGui::DragFloat("diskEdgeK", &data.diskEdgeK);
		ImGui::DragFloat("Spin", &data.spin);
		ImGui::DragFloat("height", &data.diskH);
		ImGui::DragFloat("DAMN", &data.bhRotation);
		ImGui::DragInt("substeps", &data.volSubsteps);

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
		data.aspect = float(lowResX) / float(lowResY);
		data.time = Time::time * 1.5f;
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

	void generateNoiseMap()
	{
		const int imageSize = 64;
		TextureCreateInfo info;
		info.height = imageSize;
		info.width = imageSize;
		info.depth = imageSize;
		info.usage = TextureUsage::ReadWrite;
		noiseTexture = engine::GetResourceManager().LoadEmptyTexture(info);

		noiseParams.size = imageSize;
		noiseParams.scale = 4.0f;
		noiseParams.boost = 1.5f;
		noiseParams.octaves = 4;
		noiseParams.tileSize = imageSize;
		noiseParams.lacunarity = 2.0f;
		noiseParams.gain = 0.5f;
		noiseParams.time = 0.0f;
		
		BufferDescription desc;
		desc.size = sizeof(NoiseParams);
		desc.type = BufferType::SSBO;
		noiseParamsBuffer = engine::GetBufferManager().createBuffer(desc);

		engine::GetBufferManager().updateBuffer(noiseParamsBuffer, &noiseParams, sizeof(NoiseParams));

		ComputeShader noiseShader = engine::GetResourceManager().LoadComputeShader(RES_DIR"shaders/compute/diskNoise.glsl");
		noiseDispatcher = std::make_unique<ComputeDispatcher>(noiseShader);
		noiseDispatcher->setBuffer("inputData", noiseParamsBuffer);
		noiseDispatcher->setTexture("outputImage", noiseTexture);
		int group = (imageSize + 7) / 8;  // = 32 workgroups
		noiseDispatcher->dispatchAndWait(group, group, group);

		engine::GetResourceManager().setTextureUsage(noiseTexture, TextureUsage::Read);
	}
};