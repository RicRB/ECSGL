#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "GLDef.h"
#include "Shader.h"
#include "Transform.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "Game.h"
#include "ECS/ECS.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui//imgui_impl_opengl3.h"
//#include "yaml-cpp/yaml.h"
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

    glfwSwapInterval(1);
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
        // add light component to object
        //auto scene = std::make_unique<Scene>(Scene());
        /*
        game.SetRenderer(renderer);
        game.SetScene(); // this does absolutely nothing but fuck you 
        game.Start(); 
        */
        /* Loop until the user closes the window */

        RenderSystem renderer;
        Game& game = Game::GetInstance();
        game.Start();
        Registry& registry = Registry::GetInstance();
        //Entity entity = ResourceManager::LoadEntityFromOBJ("res/models/cube.obj");
        //Transform transform = Transform(glm::vec3(0.0f,0.0f,-10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
        //
        //Shader shader = ResourceManager::LoadShader("res/shaders/Basic.vert", "res/shaders/Basic.frag")

        Entity camera = GameEngine::CreateEmptyEntity();
        registry.AssignComponent<Camera>(camera, Camera());
        Entity entity = GameEngine::CreateObject();
        Texture texture = ResourceManager::LoadTexture("res/Images/container2.png");
        registry.GetComponent<Material>(entity).diffuse = MaterialVariable(texture);
        Entity entity2 = GameEngine::CreateObject();
        Entity light = GameEngine::CreateDirLight();
        Entity pLight = GameEngine::CreatePointLight();

        GameEngine::SetCamera(camera);
        auto& transformView = registry.GetComponent<Transform>(entity);
        auto& materialView = registry.GetComponent<Material>(entity);
        Transform cameraTransform = Transform();

        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f););
            renderer.Clear();

            renderer.DrawElements();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            // we want to move this shit into game or gui render type of thing
            {
               ImGui::Begin("Hello, world!");
               ImGui::DragFloat3("Position ", &transformView.position.x);
               ImGui::DragFloat3("Scale", &transformView.scale.x);
               ImGui::DragFloat3("Rotation", &transformView.rotation.x);
               ImGui::DragFloat3("Color", &materialView.diffuse.vec3.x);
               if (ImGui::Button("Create/Destroy", ImVec2(50, 40))) {
                   registry.DestroyEntity(entity);
               }
               // Create a window called "Hello, world!" and append into it.
                //for (unsigned int i = 0; i < objects.size(); i++)
                //{
                //    std::string objectTitle = "Object #" + std::to_string(i) + " Settings";
                //    ImGui::Text(objectTitle.c_str());
                //    ImGui::DragFloat3("Position " + i, &objects[i]->GetPosition().x);
                //    ImGui::DragFloat3("Rotation " + i, &objects[i]->GetRotation().x);
                //    ImGui::DragFloat3("Scale " + i, &objects[i]->GetScale().x);
                //    //ImGui::SliderFloat3("Position " + i, &objects[i]->GetPosition().x, -480.0f, 480.0f);
                //    //ImGui::SliderFloat3("Rotation " + i, &objects[i]->GetRotation().x, -180.0f, 180.0f);
                //    //ImGui::SliderFloat3("Scale " + i, &objects[i]->GetScale().x, 0.0f, 5.0f);
                //    ImGui::NewLine();
                //}
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