//#pragma once
//
//#ifndef OBJECT_H
//#define OBJECT_H
//#include <memory>
//#include "Transform.h"
//#include "Model.h"
//#include "Material.h"
//class Renderer;
//enum class ComponentType;
//class Component;
//class Object 
//{
//private:
//	Transform m_Transform;
//	Model *mModel;
//	MaterialOld *mMaterial;
//	Shader *mShader;
//	std::vector<std::unique_ptr<Component>> mComponents;
//	//Object(const Object& other) = delete;
//public:
//	Object(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 size, Model& model, MaterialOld& material);
//	~Object();
//	//template of getcomponent
//
//	template <typename T>
//	std::unique_ptr<T>& GetComponent();
//	std::unique_ptr<Component>& GetComponent(ComponentType type);
//	std::vector<std::shared_ptr<Component>> GetComponents(ComponentType type);
//	void AddComponent(std::unique_ptr<Component>& component);
//	void DeleteComponent(Component& component);
//	void Bind(const Renderer& renderer);
//	inline const Model* GetModel() const {return mModel;}
//	inline const Shader& GetShader() const { return *mShader; }
//	glm::vec3& GetPosition() { return m_Transform.position; }
//	glm::vec3& GetRotation() { return m_Transform.rotation; }
//	glm::vec3& GetScale() { return m_Transform.scale; }
//	glm::mat4 GetModelMatrix() const;
//};
//#endif
