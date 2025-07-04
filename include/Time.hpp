#pragma once
class Time
{
public:
	static double deltaTime;
	static double time;
private:
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
private:
};

double Time::deltaTime = 0.0f;
double Time::time = 0.0f;
double Time::lastFrame = 0.0f;