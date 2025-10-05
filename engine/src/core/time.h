#pragma once
class Time
{
private:
	inline static double deltaTime = 0;
	inline static double time = 0;
	inline static double lastFrame = 0;

public:
	static void Init();
	static void Update();
	static const double getDeltaTime();
private:
};