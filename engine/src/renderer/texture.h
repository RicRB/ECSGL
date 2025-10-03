#pragma once
#include <string>
#include "image.h"
using TextureID = std::string;
enum class TextureType
{
	UNKOWN,
	DIFFUSE,
	SPECULAR
};

struct TextureData
{
	TextureData();
	TextureID id;
	unsigned char* pixels;
	int width, height, bpp;
	std::shared_ptr<TextureImage> image;
};
struct Texture
{
	 TextureID id;
	 Texture(TextureID id = "") : id(id) {}
};