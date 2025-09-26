#include "core/application.h"
#include "core/engine.h" // HOLY SHIT IT WORKS?????????????
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
	return 0;
}