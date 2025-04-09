//#include "Object.h"
//#include "Renderer.h"
//#include "Component.h"
//#define GLM_ENABLE_EXPERIMENTAL
//#include "glm/ext.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//Object::Object(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 size, Model& model, Material& material)
//	: m_Transform(position, rotation, size), mModel(&model), mMaterial(&material), mShader(material.m_Shader)
//{
//}
//
//Object::~Object()
//{
//	//for (unsigned int i = 0; i < mComponents.size(); i++)
//	//{
//	//	mComponents[i]->mObject.reset();
//	//}
//}
//template<typename T>
//std::unique_ptr<T>& Object::GetComponent()
//{
//	// iterate over mComponents to find component of type
//	for (unsigned int i = 0; i < mComponents.size(); i++)
//	{
//		if (mComponents[i]->mType == T::mType)
//			return static_cast<std::unique_ptr<T>&>(mComponents[i]);
//	}
//	// Return a null unique_ptr if the component is not found
//	static std::unique_ptr<T> nullComponent;
//	return nullComponent;
//}
//std::unique_ptr<Component>& Object::GetComponent(ComponentType type)
//{
//	// iterate over mComponents to find component of type
//	for (unsigned int i = 0; i < mComponents.size(); i++)
//	{
//		if (mComponents[i]->GetType() == type)
//			return mComponents[i];
//	}
//	// Return a null unique_ptr if the component is not found
//	static std::unique_ptr<Component> nullComponent;
//	return nullComponent;
//}
//std::vector<std::shared_ptr<Component>> Object::GetComponents(ComponentType type)
//{
//	// iterate over mComponents to find all components of type
//	std::vector<std::shared_ptr<Component>> components;
//	for (auto it = mComponents.begin(); it != mComponents.end(); it++)
//	{
//		if (it->get()->GetType() == type)
//		{
//			components.push_back(std::shared_ptr<Component>(it->get()));
//		}
//	}
//	return components;
//}
//void Object::AddComponent(std::unique_ptr<Component>& component)
//{
//	component->BindToObject(*this);
//	mComponents.push_back(std::move(component));
//}
//void Object::DeleteComponent(Component& component)
//{
//	for (auto it = mComponents.begin(); it != mComponents.end(); it++)
//	{
//		if (it->get() == &component)
//		{
//			mComponents.erase(it);
//			break;
//		}
//	}
//}
//void Object::Bind(const Renderer &renderer)
//{
//	int textureSlot = 0;
//	mMaterial->Bind(textureSlot);
//	// projection matrix
//	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 960.0f / 540.0f, 0.1f, 500.0f);
//	// view matrix is position, rotation of camera
//	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -100.0f));
//	glm::mat4 model = GetModelMatrix();
//
//	glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(model)));
//	glm::vec4 color(0.8f, 0.3f, 0.9f, 1.0f);
//	glm::vec4 globalLight(1.0f, 1.0f, 1.0f, 1.0f);
//
//	mShader->SetUniformMat4f("u_Projection", proj);
//	mShader->SetUniformMat4f("u_View", view);
//	mShader->SetUniformMat4f("u_Model", model);
//	mShader->SetUniformMat3f("u_Normal", normal);
//
//	mShader->SetUniform3f("dirLights[0].direction", 0.0f, 0.0f, 0.0f);
//	mShader->SetUniform3f("dirLights[0].ambient", 0.2f, 0.2f, 0.2f);
//	mShader->SetUniform3f("dirLights[0].diffuse", 1.0f, 0.5f, 0.5f);
//	mShader->SetUniform3f("dirLights[0].specular", 1.0f, 1.0f, 1.0f);
//
//	mShader->SetUniform3f("pointLights[0].position", 0.2f, 0.2f, 75.0f);
//	mShader->SetUniform1f("pointLights[0].constant", 1.0f);
//	mShader->SetUniform1f("pointLights[0].linear", 0.09f);
//	mShader->SetUniform1f("pointLights[0].quadratic", 0.032f);
//	mShader->SetUniform3f("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
//	mShader->SetUniform3f("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
//	mShader->SetUniform3f("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
//}
//
//glm::mat4 Object::GetModelMatrix() const
//{
//	return m_Transform.GetTransformMatrix();
//}
//
//
//
