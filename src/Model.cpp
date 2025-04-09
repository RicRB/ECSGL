//#include "Model.h"
//#include "VertexBufferLayout.h"
//#include "Game.h"
//#include <iostream>
//#include <assimp/Importer.hpp>
//#include <assimp/postprocess.h>
//
//void Model::Draw()
//{
//	for (unsigned int i = 0; i < mMeshes.size(); i++)
//	{
//		mMeshes[i]->Bind();
//	}
//}
//
//void Model::LoadModel(std::string path)
//{
//	Assimp::Importer import;
//	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate |aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
//	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//	{
//		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
//		return;
//	}
//	m_Directory = path.substr(0, path.find_last_of('/'));
//
//	ProcessNode(scene->mRootNode, scene);
//}
//
//void Model::ProcessNode(aiNode* node, const aiScene* scene)
//{
//	// process all the node's meshes (if any)
//	for (unsigned int i = 0; i < node->mNumMeshes; i++)
//	{
//		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
//		mMeshes.push_back(processMesh(mesh, scene));
//	}
//	// then do the same for each of its children
//	for (unsigned int i = 0; i < node->mNumChildren; i++)
//	{
//		ProcessNode(node->mChildren[i], scene);
//	}
//}
//
//Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene)
//{
//	std::vector<Vertex> vertices;
//	std::vector<unsigned int> indices;
//	std::vector<Material> material;
//	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//	{
//		Vertex vertex;
//		glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
//		vertex.position = position;
//		glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
//		vertex.normal = normal;
//		if (mesh->mTextureCoords[0])
//		{
//			auto length = mesh->mTextureCoords[1];
//			glm::vec2 textCoords(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
//			vertex.textCoords = textCoords;
//		}
//		else
//			vertex.textCoords = glm::vec2(0.0f, 0.0f);
//		vertices.push_back(vertex);
//	}
//	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
//	{
//		aiFace face = mesh->mFaces[i];
//		for (unsigned int j = 0; j < face.mNumIndices; j++)
//			indices.push_back(face.mIndices[j]);
//	}
//	// TODO: ADD TEXTURE LOADIGN AND MATERIAL LOADING
//	/*if (mesh->mMaterialIndex >= 0)
//	{
//		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material,
//			aiTextureType_DIFFUSE);
//
//		std::vector<Texture> specularMaps = LoadMaterialTextures(material,
//			aiTextureType_SPECULAR);
//	}*/
//	VertexArray* va = new VertexArray();
//	VertexBuffer vb(vertices.data(), vertices.size() * sizeof(Vertex));
//	VertexBufferLayout layout;
//	layout.Push<Vertex>(1);
//	va->AddBuffer(vb, layout);
//	IndexBuffer* ib = new IndexBuffer(indices.data(), indices.size());
//	Mesh *newMesh = new Mesh(*va,*ib);
//	return newMesh;
//}
//
//std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type)
//{
//	std::vector<Texture> textures;
//	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//	{
//		aiString str;
//		mat->GetTexture(type, i, &str);
//		Texture texture(str.C_Str());
//		if (type == aiTextureType_DIFFUSE)
//			texture.textureType = TextureType::DIFFUSE;
//		else if (type == aiTextureType_SPECULAR)
//			texture.textureType = TextureType::SPECULAR;
//		textures.push_back(texture);
//	}
//	return textures;
//}
