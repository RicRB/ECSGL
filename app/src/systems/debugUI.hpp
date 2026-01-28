#include "ecs/ecs.hpp"
#include "core/engine.h"
#include "core/time.h"
#include "core/input.h"
#include "imgui/imgui.h"
class DebugUI : public System {
public:
	float fps = 0.0f;
	float times = 0.0f;
	int frame = 0;

	DebugUI() = default;
	void start() override {
	}
	void update() override {
	}
	void onGui() override {
		times += 1.0f;
		if (times >= 1000)
		{
			times = 0;
			fps = 0.0f;
		}

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		fps = fps + (1000.0f / io.Framerate);
		ImGui::Begin("Hello, world!");
		ImGui::Text("Frame : %d", frame);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::Text("Average Frame time : %.3f ms", fps / times);
		frame += 1;

		ImGui::End();
	}
	void cleanup() override {
		// Cleanup code if needed
	}
	const char* getName() const override {
		return "DebugUI";
	}
};