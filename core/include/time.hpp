#pragma once
#include "gl_def.h"
class Time
{
private:
	static double deltaTime;
	static double time;
	static double lastFrame;

public:
	static void Init()
	{
		deltaTime = 0.0f;
		time = 0.0f;
		lastFrame = 0.0f;
	}
	static void Update()
	{
		time = glfwGetTime();
		deltaTime = time - lastFrame;
		lastFrame = time;
	}
	static const double getDeltaTime()
	{
		return deltaTime;
	}
private:
};