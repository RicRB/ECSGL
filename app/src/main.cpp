#include "core/application.h"
#include "core/engine.h" // HOLY SHIT IT WORKS?????????????
#include "systems/camera_movement.hpp"
#include "systems/raycaster.hpp"
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
    currentEngine.registry.AddSystem<Raycaster>();



    Transform transform;
    transform.setPosition(glm::vec3(0, 0, -10.0f));
    transform.setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
    transform.setRotation(glm::vec3(45.0f, 45.0f, 0.0f));

    for (size_t i = 0; i < 0; i++)
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

	Texture text = currentEngine.resourceManager.LoadTexture(RES_DIR"images/container2.png", TextureUsage::Read);

    currentEngine.resourceManager.bindTextureToMaterial(currentEngine.defaultMaterial, text, 0);

    game.Start();
	return 0;
}