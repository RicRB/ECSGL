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
	Texture blueNoiseTexture;
	int textureUsedByShader = 0;
	int fullResX = 1270, fullResY = 720;
	float lowResRatio = 0.35f;
	int threadGroupX = 16, threadGroupY = 16;
	int renderThreadGroupX = 16, renderThreadGroupY = 16;
	uint32_t groupCountX, groupCountY;
	uint32_t renderGroupCountX, renderGroupCountY;
	std::unique_ptr<ComputeDispatcher> dispatcher;
	std::unique_ptr<ComputeDispatcher> renderDispatcher;

	int currentPass = 0;
	struct BlackHoleData {
		glm::vec3 pos; float _pad0;
		glm::vec3 right; float _pad1;
		glm::vec3 up; float _pad2;
		glm::vec3 forward; float _pad3;
		float tanHalfFov;
		float aspect;
		int maxSteps = 150;
		float stepSizeMin = 0.1f;
		float stepSizeMax = 5.0f;
		float rotSpeed = 0.5f;
		float mass = 1.0f;
		float diskInnerRadius = 5.0f;
		float diskOuterRadius = 12.0f;
		float diskHeight = 0.4f;
		float diskDensity = 5.0f;
		float expoFactor = 1.0f;
		float aniForward = 0.6;
		float aniBackward = -0.2;
		float lobeWeight = 0.3;
		float brightness = 0.4;
		float lightIntensity = 1000;
		float rotationSpeed = -25;
		float verticalFadeStart = 0.3;
		float verticalFadeEnd = 0.56;
		float outerFadeStart = 2;
		float outerFadeEnd = 5.91;
		float innerFadeRadius = 0.79;
		float innerFadeWidth = 1;
		float lightFallOff = 0.9f;
		float dopplerStrength = 0.9;

		int shadowIterations = 3;
		float shadowStepSize = 2.5;

		float dsVolScale = 1.0;
		int volSubsteps = 20;
		float time = 0.0f;
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
		int size = 64;
		float scale = 1.0f;
		float boost = 2.0f;
		int octaves = 6;
		int tileSize = 64;
		float lacunarity = 2.0f;
		float gain = 0.5f;
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
		entity = engine::CreateObject(Transform(glm::vec3(), glm::vec3(0.0,180.0,0.0)), mesh, materials[1]);


		// TEXTURES
		{
			TextureCreateInfo lowResInfo;
			lowResInfo.width = glm::ceil(fullResX * lowResRatio);
			lowResInfo.height = glm::ceil(fullResY * lowResRatio);
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

		sphereMap = engine::GetEngine().resourceManager.LoadTexture(RES_DIR"images/nebula.png", TextureUsage::Read);
		blueNoiseTexture = engine::GetResourceManager().LoadTexture(RES_DIR"images/blueNoise.png", TextureUsage::Read);

		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[0], textures[0], 0);
		engine::GetEngine().resourceManager.bindTextureToMaterial(materials[1], textures[1], 0);


		auto& camTransform = registry.GetComponent<Transform>(camera);
		camTransform.setCaching(false);
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

		groupCountX = (glm::ceil(fullResX * lowResRatio) + threadGroupX -1) / threadGroupX;
		groupCountY = (glm::ceil(fullResY * lowResRatio) + threadGroupY -1) / threadGroupY;

		dispatcher->setBuffer("inputData", buffer);
		dispatcher->setBuffer("objectData", objectBuffer);
		dispatcher->setTexture("noiseMap", noiseTexture);
		dispatcher->setTexture("directionTexture", directionTexture);
		dispatcher->setTexture("volumeTexture", volumeTexture);
		dispatcher->setTexture("blueNoiseTexture", blueNoiseTexture);
		dispatcher->dispatch(groupCountX, groupCountY, 1);

		currentPass = 0;
		textureUsedByShader = 1;


		ComputeShader renderShader = engine::GetResourceManager().LoadComputeShader(RES_DIR"shaders/compute/blackHoleRender.glsl");
		renderDispatcher = std::make_unique<ComputeDispatcher>(renderShader);

		renderGroupCountX = (fullResX + renderThreadGroupX-1) / renderThreadGroupX;
		renderGroupCountY = (fullResY + renderThreadGroupY-1) / renderThreadGroupY;
		
		renderDispatcher->setBuffer("inputData", buffer);
		renderDispatcher->setBuffer("objectData", objectBuffer);
		renderDispatcher->setTexture("noiseMap", noiseTexture);
		renderDispatcher->setTexture("blueNoiseTexture", blueNoiseTexture);
		renderDispatcher->setTexture("directionTexture", directionTexture);
		renderDispatcher->setTexture("volumeTexture", volumeTexture);
		renderDispatcher->setTexture("sphereMapTexture", sphereMap);
		renderDispatcher->setTexture("outputImage", textures[textureUsedByShader]);
		//renderDispatcher->dispatch(renderGroupCountX, renderGroupCountY, 1);
	}
	void update() override {
		HandleInputs();
		switch (currentPass)
		{
		case(0): // WAIT FOR RAYMARCHING PASS TO BE DONE
			if (!dispatcher->isDone()) return;
			currentPass = (currentPass + 1) % 2;
			textureUsedByShader = (textureUsedByShader + 1) % 2;

			updateData();

			renderDispatcher->setTexture("outputImage", textures[textureUsedByShader]);
			renderDispatcher->dispatch(renderGroupCountX, renderGroupCountY, 1);
			break;
		case(1): // WAIT FOR RENDER PASS TO BE DONE
			if (!renderDispatcher->isDone()) return;
			currentPass = (currentPass + 1) % 2;

			updateData();

			engine::GetRegistry().AssignComponent<Material>(entity, materials[textureUsedByShader]);
			dispatcher->dispatch(groupCountX, groupCountY, 1);
			break;
		}
	}

	void onGui() override {
		ImGui::Begin("BLACK HOLE");
		ImGui::DragFloat("stepSizeMin", &data.stepSizeMin);
		ImGui::DragFloat("stepSizeMax", &data.stepSizeMax);
		ImGui::DragInt("maxSteps", &data.maxSteps);
		ImGui::DragFloat("Rotation Speed", &data.rotSpeed, 0.1, 0, 5);
		ImGui::DragFloat("Mass", &data.mass);
		ImGui::End();

		ImGui::Begin("DISK");
		ImGui::DragFloat("Density", &data.diskDensity, 0.1f);
		ImGui::DragFloat("InnerRingRadius", &data.diskInnerRadius);
		ImGui::DragFloat("OuterRingRadius", &data.diskOuterRadius);
		ImGui::DragFloat("HeightRing", &data.diskHeight);
		ImGui::DragFloat("Brightness", &data.brightness);
		ImGui::DragFloat("LightIntensity", &data.lightIntensity);
		ImGui::DragFloat("RotationSpeed", &data.rotationSpeed);
		ImGui::DragFloat2("Vertical Fade", &data.verticalFadeStart, 0.1f);
		ImGui::DragFloat2("OuterFade", &data.outerFadeStart, 0.1f);
		ImGui::DragFloat("InnerFade Radius", &data.innerFadeRadius, 0.1f);
		ImGui::DragFloat("Light fall off", &data.lightFallOff, 0.1f);
		ImGui::DragInt("Substeps", &data.volSubsteps);
		ImGui::DragFloat("SUBSTEP SCALE??", &data.dsVolScale, 0.1f);
		ImGui::DragFloat("Shadow", &data.shadowStepSize);
		ImGui::DragInt("ShadowIt", &data.shadowIterations);
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
		data.aspect = float(fullResX) / float(fullResY);
		data.time = Time::time * 1.5f;

		engine::GetBufferManager().updateBuffer(buffer, &data, sizeof(BlackHoleData));
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