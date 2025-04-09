#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Object.h"
#include "Lights.h"
#include <vector>
#include <memory>
#include "GLDef.h"


#define GLC_VERTEX 2023
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textCoords;
};


#include <vector>
class Renderer
{
private:
    std::vector<std::shared_ptr<Light>> lights;
public:
    Renderer();
    ~Renderer();
    void Init();
    //void BindLights(Shader& shader) const;
    void Draw(const VertexArray& vertexArray, const IndexBuffer& indexBuffer, const Shader& shader) const;
    void Draw(Object& object) const;
    void DrawObjects(std::vector<Object*>& objects) const;
    void Clear() const;
    std::vector<std::shared_ptr<Light>>& GetLights() { return lights; }
private:
    //void FetchLights();
};
#endif