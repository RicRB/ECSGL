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

	ComputeShader shader = currentEngine.resourceManager.LoadComputeShader(RES_DIR"shaders/compute/test.glsl");
	ComputeDispatcher dispatcher(shader);
	Texture text = currentEngine.resourceManager.LoadTexture(RES_DIR"images/container2.png", TextureUsage::Read);
	Texture text2 = currentEngine.resourceManager.LoadEmptyTexture(500, 500, TextureUsage::ReadWrite);

	dispatcher.setTexture("inputImage", &text);
	dispatcher.setTexture("outputImage", &text2);

    uint32_t groupCountX = (500 + 15) / 16;  // = 32 workgroups
    uint32_t groupCountY = (500 + 15) / 16;  // = 32 workgroups
	dispatcher.dispatch(groupCountX, groupCountY, 1);
    dispatcher.wait();

    currentEngine.resourceManager.setTextureUsage(text2, TextureUsage::Read);
    currentEngine.resourceManager.bindTextureToMaterial(currentEngine.defaultMaterial, text2, 0);

    game.Start();
	return 0;
}