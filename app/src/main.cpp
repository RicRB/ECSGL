#include "core/application.h"
#include "core/engine.h" // HOLY SHIT IT WORKS?????????????
#include "systems/camera_movement.hpp"
#include "renderer/compute_dispatcher.h"
class Game : public Core::Application {
public:
	Game() : Core::Application() {

	}

	~Game() {
	}
};
int main(void)
{
	Game game;
    Engine& currentEngine = engine::GetEngine();
    currentEngine.registry.AddSystem<CameraMovementSystem>();

    Transform transform;
    transform.setPosition(glm::vec3(0, 0, -10.0f));
    transform.setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
    transform.setRotation(glm::vec3(45.0f, 45.0f, 0.0f));

    for (size_t i = 0; i < 100; i++)
    {
        glm::vec3 hey = glm::vec3(std::fmod((static_cast<float>(i) * 2.0f), 20.0f), static_cast<float>(glm::floor(i / 20)), -10.0f);
        transform.setPosition(hey);
        Entity entity = engine::CreateDefaultObject(transform);
    }
    //Entity entity = engine::CreateDefaultObject(transform);
    Entity light = engine::CreateDirLight();

    Transform cT = currentEngine.registry.GetComponent<Transform>(currentEngine.currentCameraEntity);
    Entity pLight = engine::CreatePointLight(cT);
    currentEngine.registry.scenegraph.ParentEntity(pLight, currentEngine.currentCameraEntity);
    Texture text = currentEngine.resourceManager.LoadTexture(RES_DIR"images/container2.png");
    currentEngine.resourceManager.bindTextureToMaterial(currentEngine.defaultMaterial, text, 0);

	ComputeShader shader = currentEngine.resourceManager.LoadComputeShader(RES_DIR"shaders/compute/test.glsl");
	ComputeDispatcher dispatcher(shader);
    std::unique_ptr<Buffer> inBuffer = Buffer::CreateUnique();
    std::unique_ptr<Buffer> outBuffer = Buffer::CreateUnique();
	BufferDesc desc;
	desc.size = 5 * sizeof(int);
	desc.type = BufferType::SSBO;


    currentEngine.bufferManager->get()->createBuffer(inBuffer.get(), desc);
	currentEngine.bufferManager->get()->createBuffer(outBuffer.get(), desc);

	int data[5] = { 1, 2, 3, 4, 5 };
	currentEngine.bufferManager->get()->updateBuffer(inBuffer.get(), data, desc.size);
	currentEngine.bufferManager->get()->updateBuffer(outBuffer.get(), data, desc.size);

	dispatcher.setBuffer("inputData", inBuffer.get(), desc.size);
	dispatcher.setBuffer("outputData", outBuffer.get(), desc.size);

	dispatcher.dispatch(5, 1, 1);
    dispatcher.wait();

    int out[5];
	currentEngine.bufferManager->get()->getBufferData(outBuffer.get(), out);

    game.Start();
	return 0;
}