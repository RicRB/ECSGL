#include "RenderSystem.h"
#include "SceneGraph.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Lights.h"
RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem()
{
}

void RenderSystem::Run()
{
	DrawElements();
}

void RenderSystem::DrawElements()
{
	Registry& registry = Registry::GetInstance();
	currentView = registry.GetView<Transform, Mesh, Material>();
	for (size_t i = 0; i < currentView.size(); i++)
	{
		auto entity = currentView[i];
		int textureIndex = 0;

		///// BIND MESH /////
		// bind vertex array
		GLCall(glBindVertexArray(std::get<2>(entity)->vertexID));
		// bind index buffer
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, std::get<2>(entity)->indexID));

		//// BIND MATERIAL ////
		// bind shader
		unsigned int shaderID = std::get<3>(entity)->shaderID;
		GLCall(glUseProgram(shaderID));

		// texture/color uniforms for material
		Material* material = std::get<3>(entity);
		if (material->diffuse.type == VariableType::TEXTURE)
		{
			shader::SetUniform1b(shaderID, "u_Material.diffuseIsMap", true);
			GLCall(glActiveTexture(GL_TEXTURE0 + textureIndex));
			GLCall(glBindTexture(GL_TEXTURE_2D, material->diffuse.texture.rendererID));
			textureIndex++;
		}
		else
		{
			shader::SetUniform1b(shaderID, "u_Material.diffuseIsMap", false);
			shader::SetUniform3f(shaderID, "u_Material.diffuse", material->diffuse.vec3.x, material->diffuse.vec3.y, material->diffuse.vec3.z);
		}
		// Specular Uniform
		if (material->specular.type == VariableType::TEXTURE)
		{
			shader::SetUniform1b(shaderID, "u_Material.specularIsMap", true);
			GLCall(glActiveTexture(GL_TEXTURE0 + textureIndex));
			GLCall(glBindTexture(GL_TEXTURE_2D, material->specular.texture.rendererID));
			textureIndex++;
		}
		else
		{
			shader::SetUniform1b(shaderID, "u_Material.specularIsMap", false);
			shader::SetUniform3f(shaderID, "u_Material.specular", material->specular.vec3.x, material->specular.vec3.y, material->specular.vec3.z);
		}


		// SETTING MATRIX UNIFORMS CAMERA & OBJECT

		// fetch camera and transform information
		Entity camera = GameEngine::GetCameraEntity();
		Transform cameraTransform = registry.GetComponent<Transform>(camera);
		
		glm::mat4 proj = GameEngine::GetCamera().projection;
		// manual camera for now
		//glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10.0f));
		glm::mat4 view = cameraTransform.viewMatrix();
		Transform* entityTransform = std::get<1>(entity);
		glm::mat4x4 localToWorld = Scenegraph::GetLocalToWorldMatrix(std::get<0>(entity));
		glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(localToWorld * entityTransform->matrix())));

		shader::SetUniformMat4f(shaderID, "u_Projection", proj);
		shader::SetUniformMat4f(shaderID, "u_View", view);
		shader::SetUniformMat4f(shaderID, "u_Model", localToWorld * entityTransform->matrix());
		shader::SetUniformMat3f(shaderID, "u_Normal", normal);
		
		// SETTING LIGHT UNIFORMS
		auto dirView = registry.GetView<DirectionalLight>();
		auto pointView = registry.GetView<PointLight, Transform>();

		for (size_t i = 0; i < dirView.size(); i++)
		{
			auto dirLight = std::get<1>(dirView[i]);
			std::string index = std::to_string(i);
			shader::SetUniform3f(shaderID, "dirLights[" + index + "].direction", dirLight->direction.x, dirLight->direction.y, dirLight->direction.z);
			shader::SetUniform3f(shaderID, "dirLights[" + index + "].ambient", dirLight->ambient.x, dirLight->ambient.y, dirLight->ambient.z);
			shader::SetUniform3f(shaderID, "dirLights[" + index + "].diffuse", dirLight->diffuse.x, dirLight->diffuse.y, dirLight->diffuse.z);
			shader::SetUniform3f(shaderID, "dirLights[" + index + "].specular", dirLight->specular.x, dirLight->specular.y, dirLight->specular.z);
		}

		for (size_t i = 0; i < pointView.size(); i++)
		{
			auto pointLight = std::get<1>(pointView[i]);
			auto transform = std::get<2>(pointView[i]);
			std::string index = std::to_string(i);
			shader::SetUniform3f(shaderID, "pointLights[" + index + "].position", transform->position.x, transform->position.y, transform->position.z);
			shader::SetUniform1f(shaderID, "pointLights[" + index + "].constant", pointLight->constant);
			shader::SetUniform1f(shaderID, "pointLights[" + index + "].linear", pointLight->linear);
			shader::SetUniform1f(shaderID, "pointLights[" + index + "].quadratic", pointLight->quadratic);
			shader::SetUniform3f(shaderID, "pointLights[" + index + "].ambient", pointLight->ambient.x, pointLight->ambient.y, pointLight->ambient.z);
			shader::SetUniform3f(shaderID, "pointLights[" + index + "].diffuse", pointLight->diffuse.x, pointLight->diffuse.y, pointLight->diffuse.z);
			shader::SetUniform3f(shaderID, "pointLights[" + index + "].specular", pointLight->specular.x, pointLight->specular.y, pointLight->specular.z);
		}

		GLCall(glDrawElements(GL_TRIANGLES, std::get<2>(entity)->count, GL_UNSIGNED_INT, nullptr));
	}
}

void RenderSystem::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
