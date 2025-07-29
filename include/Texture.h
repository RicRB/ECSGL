#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>

using TextureID = unsigned int;
enum class TextureType
{
	UNKOWN,
	DIFFUSE,
	SPECULAR
};

struct TextureData
{
	TextureID id;
	unsigned char* pixels;
	int width, height, bpp;
};
struct Texture
{
	 TextureID id;
	 Texture(TextureID id = 0) : id(id) {}
};
#endif