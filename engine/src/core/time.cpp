#include "time.h"
#include "GLFW/glfw3.h"
double Time::deltaTime = 0.0f;
double Time::time = 0.0f;
double Time::lastFrame = 0.0f;

void Time::Init()
{
	deltaTime = 0.0f;
	time = 0.0f;
	lastFrame = 0.0f;
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