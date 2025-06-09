#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "gl_def.h"
#include "shader.h"
#include "transform.h"
#include "renderer.h"
#include "resource_manager.h"
#include "engine.h"
#include "ecs/ecs.hpp"
#include "input.hpp"
#include "time.hpp"
#include "event.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui//imgui_impl_opengl3.h"
#include "renderer/vulkan.h"
//#include "yaml-cpp/yaml.h"
void PrintMessage(int i) {
    std::cout << i << "\n";
}


#define GLM_ENABLE_EXPERIMENTAL
int main(void)
{
    // hi
    GLFWwindow* window;
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 540, "Open GL Time", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);
    glewInit();

    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR!" << std::endl;
    }
    std::cout << glGetString(GL_VERSION) << std::endl;

    {
        glEnable(GL_BLEND);
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        ImGui::CreateContext();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        bool show_demo_window = false;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // setup Input system
        Input::Init(window);
		Time::Init();
        /* Loop until the user closes the window */

        VulkanRenderer vulkan;
        vulkan.init();



        Renderer renderer;
        Engine game;
        engine::SetCurrentEngine(game);
        game.Start();
        Event<int> event;
		event.AddListener(PrintMessage);
		event.RemoveListener(PrintMessage);
		event.Invoke(5);
        Registry& registry = game.registry;
        Entity camera = engine::CreateEmptyEntity();
        registry.AssignComponent<Camera>(camera, Camera());
        registry.AssignComponent<Transform>(camera, Transform());

        Entity entity = engine::CreateObject();
        Texture texture = game.resourceManager.LoadTexture("res/Images/container2.png");
        registry.GetComponent<Material>(entity).diffuse = MaterialVariable(texture);
        Entity entity2 = engine::CreateObject(Transform(glm::vec3(-1.0f, 0,0)));
		game.registry.scenegraph.ParentEntity(entity2, entity);
        Entity light = engine::CreateDirLight();
        Entity pLight = engine::CreatePointLight();

        engine::SetCamera(camera);
        auto& transformView = registry.GetComponent<Transform>(entity);
        auto& transformView2 = registry.GetComponent<Transform>(entity2);
        auto& materialView = registry.GetComponent<Material>(entity);
		auto& cameraTransformView = registry.GetComponent<Transform>(camera);
		cameraTransformView.position = glm::vec3(0.0f, 0.0f, -10.0f);

        double currentFrame;
		double lastFrame = 0.0f;
		double deltaTime = 0.0f;
        while (!glfwWindowShouldClose(window))
        {
            GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f););
            renderer.Clear();

            Time::Update();

			if (Input::GetKeyPressed(Key::A))
			{
				cameraTransformView.position.x -= 10.0f * Time::deltaTime;
			}
            if (Input::GetKeyPressed(Key::D))
            {
                cameraTransformView.position.x += 10.0f * Time::deltaTime;
            }
            renderer.Update(registry);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            // GUI
            {
               ImGui::Begin("Hello, world!");
               ImGui::DragFloat3("Position ", &transformView.position.x);
               ImGui::DragFloat3("Scale", &transformView.scale.x);
               ImGui::DragFloat3("Rotation", &transformView.rotation.x);
               ImGui::DragFloat3("Color", &materialView.diffuse.vec3.x);
               ImGui::DragFloat3("camera Position", &cameraTransformView.position.x);
               ImGui::DragFloat3("Position2", &transformView2.position.x);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();
            }
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}