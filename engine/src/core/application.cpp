#include "application.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include "renderer/shader.h"
#include "components/transform.h"
#include "resource_manager.h"
#include "engine.h"
#include "ecs/ecs.hpp"
#include "input.h"
#include "event.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "platform/vulkan/vulkan.h"
#include "renderer/graphics_api.h"
#include "time.h"
void PrintMessage(int i) {
    std::cout << i << "\n";
}
namespace Core {

	Application::Application() : engine(GRAPHICS_API_VULKAN)
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }
		window = std::make_shared<Window>();
		window->create();
        if (!window->getHandle())
        {
            glfwTerminate();
			throw std::runtime_error("Failed to create window");
        }

        engine.initialize(window);
        Input::Init(window);
        Time::Init();
    }

	Application::~Application()
	{
        engine.cleanup();
        window->destroy();
        glfwTerminate();
	}

    void Application::Start()
    {
            engine.Start();

            while (!glfwWindowShouldClose(window->getHandle()))
            {
                glfwPollEvents();

                engine.Update();

                window->update();
            }
    }
}