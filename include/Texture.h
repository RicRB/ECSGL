#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>

enum class TextureType
{
	UNKOWN,
	DIFFUSE,
	SPECULAR
};

//class Texture
//{
//private:
//	unsigned int m_RendererID;
//	std::string m_FilePath;
//	unsigned char* m_LocalBuffer;
//	int m_Width, m_Height, m_BPP;
//public:
//	TextureType textureType;
//	Texture(const std::string path);
//	~Texture();
//
//	void Bind(unsigned int slot = 0) const;
//	void Unbind() const;
//
//	inline int GetWidth() const { return m_Width; }
//	inline int GetHeight() const { return m_Height; }
//};
//

struct TextureData
{
	unsigned int mRendererID;
	unsigned char* mLocalBuffer;
	int mWidth, mHeight, mBPP;
};
struct Texture
{
	 unsigned int rendererID;
	Texture(unsigned int ID)
		: rendererID(ID) 
	{}
};
#endif