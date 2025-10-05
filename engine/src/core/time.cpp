#include "time.h"
#include "GLFW/glfw3.h"

void Time::Init()
{
	deltaTime = 0.0;
	time = 0.0;
	lastFrame = 0.0;
}
void Time::Update()
{
	time = glfwGetTime();
	deltaTime = time - lastFrame;
	lastFrame = time;
}
const double Time::getDeltaTime()
{
	return deltaTime;
}