#include "core/application.h"
#include "core/engine.h" // HOLY SHIT IT WORKS?????????????
#include "systems/camera_movement.hpp"
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

    engine::currentEngine->registry.AddSystem<CameraMovementSystem>();

    Transform transform;
    transform.setPosition(glm::vec3(0, 0, -10.0f));
    transform.setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
    transform.setRotation(glm::vec3(45.0f, 45.0f, 0.0f));
    for (size_t i = 0; i < 900; i++)
    {
        glm::vec3 hey = glm::vec3(std::fmod((static_cast<float>(i) * 2.0f), 20.0f), static_cast<float>(glm::floor(i / 20)), -10.0f);
        transform.setPosition(hey);
        Entity entity = engine::CreateDefaultObject(transform);
    }
    //Entity entity = engine::CreateDefaultObject(transform);
    Entity light = engine::CreateDirLight();
    Entity pLight = engine::CreatePointLight();

    engine::currentEngine->registry.scenegraph.ParentEntity(pLight, engine::currentEngine->currentCameraEntity);

    game.Start();
	return 0;
}