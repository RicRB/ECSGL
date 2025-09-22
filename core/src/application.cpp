#include "application.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include "shader.h"
#include "transform.h"
#include "resource_manager.h"
#include "engine.h"
#include "ecs/ecs.hpp"
#include "input.hpp"
#include "event.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "renderer/vulkan.h"
#include "renderer/graphics_api.h"
#include "systems/camera_movement.hpp"
void PrintMessage(int i) {
    std::cout << i << "\n";
}

int Application::Init(void)
{
    const GraphicsAPIType graphicsAPIType = GRAPHICS_API_VULKAN; // Change to GRAPHICS_API_VULKAN for Vulkan
    // hi
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    if (graphicsAPIType == GRAPHICS_API_OPENGL)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS compatibility
    }
    else if (graphicsAPIType == GRAPHICS_API_VULKAN)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Prevents OpenGL context creation
    }
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 540, "Vulkan Time", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    /* Make the window's context current */
    //glfwMakeContextCurrent(window);

    // glewInit();

 /*if (glewInit() != GLEW_OK)
 {
     std::cout << "ERROR!" << std::endl;
 }
 std::cout << glGetString(GL_VERSION) << std::endl;*/
    Engine engine(graphicsAPIType);

    {

        engine.initialize(window);
        Input::Init(window);
        Time::Init();
        /*glEnable(GL_BLEND);
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);*/

        // setup Input system
  //      Input::Init(window);
        //Time::Init();
  //      /* Loop until the user closes the window */



  //      Renderer renderer;
  //      Engine game;
  //      engine::SetCurrentEngine(game);
  //      game.Start();
  //      Event<int> event;
        //event.AddListener(PrintMessage);
        //event.RemoveListener(PrintMessage);
        //event.Invoke(5);
  //      Registry& registry = game.registry;
  //      Entity camera = engine::CreateEmptyEntity();
  //      registry.AssignComponent<Camera>(camera, Camera());
  //      registry.AssignComponent<Transform>(camera, Transform());

  //      Entity entity = engine::CreateObject();
  //      Texture texture = game.resourceManager.LoadTexture("res/Images/container2.png");
  //      registry.GetComponent<Material>(entity).diffuse = MaterialVariable(texture);
  //      Entity entity2 = engine::CreateObject(Transform(glm::vec3(-1.0f, 0,0)));
        //game.registry.scenegraph.ParentEntity(entity2, entity);
  //      Entity light = engine::CreateDirLight();
  //      Entity pLight = engine::CreatePointLight();

  //      engine::SetCamera(camera);
  //      auto& transformView = registry.GetComponent<Transform>(entity);
  //      auto& transformView2 = registry.GetComponent<Transform>(entity2);
  //      auto& materialView = registry.GetComponent<Material>(entity);
        //auto& cameraTransformView = registry.GetComponent<Transform>(camera);
        //cameraTransformView.position = glm::vec3(0.0f, 0.0f, -10.0f);

  //      double currentFrame;
        //double lastFrame = 0.0f;
        //double deltaTime = 0.0f;
        engine.registry.AddSystem<CameraMovementSystem>();
        engine.Start();


        Transform transform;
        transform.setPosition(glm::vec3(0, 0, -10.0f));
        transform.setScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
        transform.setRotation(glm::vec3(45.0f, 45.0f, 0.0f));
        for (size_t i = 0; i < 900; i++)
        {
            glm::vec3 hey = glm::vec3(std::fmod((static_cast<float>(i) * 2.0f), 20.0f), static_cast<float>(glm::floor(i / 20)), -10.0f);
            transform.setPosition(hey);
            Entity entity = engine::CreateDefaultObject(transform);
        }
        //Entity entity = engine::CreateDefaultObject(transform);
        Entity light = engine::CreateDirLight();
        Entity pLight = engine::CreatePointLight();

        engine.registry.scenegraph.ParentEntity(pLight, engine.currentCameraEntity);
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            engine.Update();

            glfwSwapBuffers(window);
        }
    }
    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();
    if (engine.renderer.rendererType == GRAPHICS_API_VULKAN)
    {
        vkDeviceWaitIdle(engine.renderer.graphicsAPI->device);
    }
    engine.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
