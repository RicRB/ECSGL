#pragma once
#ifndef MODEL_H
#define MODEL_H


#include <vector>
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
//class Model
//{
//public:
//	std::vector<Mesh*> mMeshes;
//	std::vector<Material*> mMaterials;
//	Model(std::string path)
//	{
//		LoadModel(path);
//	}
//	void Draw();
//private:
//	std::string m_Directory;
//	Model(const Model& other) = delete;
//	void LoadModel(std::string path);
//	void ProcessNode(aiNode *node, const aiScene* scene);
//	Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
//	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
//};
struct Model
{
public:
	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;
};
struct ModelData
{
	std::string path;
};
#endif // !MODEL_H