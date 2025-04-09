//#include "Renderer.h"
//#include "Game.h"
//#include <iostream>
//
//// Clears errors
//void GLClearError()
//{
//    while (glGetError() != GL_NO_ERROR);
//}
//// Returns whether it succeeded or nah, while loop shouldnt rly exist but im not gon bother
//bool GLLogCall(const char* function, const char* file, int line)
//{
//    while (GLenum error = glGetError())
//    {
//        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
//        return false;
//    }
//    return true;
//}
//
//
//
//
//// RENDERER CLASS //
//Renderer::Renderer()
//{
//}
//
//Renderer::~Renderer()
//{
//}
//
//void Renderer::Init()
//{
//    //FetchLights();
//}
//
////void Renderer::BindLights(Shader& shader) const
////{
////for(unsigned int i = 0; i < mLights.size(); i++)
////	{
////    Light* newLight = std::dynamic_pointer_cast<Light*>(*mLights[i].get());
////    switch (newLight->mLightType)
////    {
////        case LightType::DIRECTIONAL:
////		{
////            // make light into directional light cast
////            DirectionalLight *light = static_cast<DirectionalLight*>(newLight);
////            // make i into a string
////
////			shader.SetUniform3f("dirLights["+ std::to_string(i) + "].direction", light->direction.x, light->direction.y, light->direction.z);
////			shader.SetUniform3f("dirLights[" + std::to_string(i) + "].ambient", light->ambient.x, light->ambient.y, light->ambient.z);
////			shader.SetUniform3f("dirLights[" + std::to_string(i) + "].diffuse", light->diffuse.x, light->diffuse.y, light->diffuse.z);
////			shader.SetUniform3f("dirLights[" + std::to_string(i) + "].specular", light->specular.x, light->specular.y, light->specular.z);
////			break;
////		}
////        case LightType::POINT:
////        {
////            // make light into point light cast
////            PointLight* light = static_cast<PointLight*>(newLight);
////            // make i into a string
////            shader.SetUniform3f("pointLights[" + std::to_string(i) + "].position", light->position.x, light->position.y, light->position.z);
////            shader.SetUniform3f("pointLights[" + std::to_string(i) + "].ambient", light->ambient.x, light->ambient.y, light->ambient.z);
////            shader.SetUniform3f("pointLights[" + std::to_string(i) + "].diffuse", light->diffuse.x, light->diffuse.y, light->diffuse.z);
////            shader.SetUniform3f("pointLights[" + std::to_string(i) + "].specular", light->specular.x, light->specular.y, light->specular.z);
////            break;
////        }
////    default:
////        break;
////    }
////	}
////}
//
//void Renderer::Draw(const VertexArray& vertexArray, const IndexBuffer& indexBuffer, const Shader& shader) const
//{
//    vertexArray.Bind();
//    indexBuffer.Bind();
//    GLCall(glDrawElements(GL_TRIANGLES, indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr));
//}
//void Renderer::Draw(Object& object) const
//{
//    object.Bind(*this);
//    std::vector<Mesh*> meshes = object.GetModel()->mMeshes;
//    for (unsigned int i = 0; i < meshes.size(); i++)
//    {
//        meshes[i]->Bind();
//        unsigned int vertexCount = meshes[i]->GetVertexCount();
//        GLCall(glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, nullptr));
//    }
//        
//}
//void Renderer::DrawObjects(std::vector<Object*>& objects) const
//{
//    for(Object* object : objects)
//    {
//        Draw(*object);
//    }
//}
//
//void Renderer::Clear() const
//{
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//}
//// get all lights in the active scene must run this every time a new scene is added or add manually
////void Renderer::FetchLights()
////{
////    Game& game = Game::GetInstance();
////    for(auto& object : game.GetActiveScene().GetObjects())
////    {
////        auto& light = object->GetComponent<Light>();
////        if(light)
////        {
////            mLights.push_back(light);
////        }
////    }
////}
