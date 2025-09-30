#pragma once
class Time
{
private:
	static double deltaTime;
	static double time;
	static double lastFrame;

public:
	static void Init();
	static void Update();
	static const double getDeltaTime();
private:
};