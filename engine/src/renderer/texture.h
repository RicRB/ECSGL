#pragma once
#include <string>
#include "image.h"
#include "memory/memory_pool.hpp"


enum class TextureType
{
	UNKOWN,
	DIFFUSE,
	SPECULAR
};

struct TextureData
{
	TextureData();
	unsigned char* pixels;
	int width, height, bpp;
	std::shared_ptr<TextureImage> image;
};
using TextureDataHandle = MemoryHandle;
struct Texture
{
	 TextureDataHandle handle;
	 Texture(TextureDataHandle h = TextureDataHandle()) : handle(h) {};
};