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

enum class TextureUsage {
	Read,
	ReadWrite
};


struct TextureData
{
	TextureData();
	unsigned char* pixels;
	int width, height, bpp;
	TextureUsage usage;
	std::shared_ptr<Image> image;
};

using TextureDataHandle = MemoryHandle;

struct Texture
{
	 TextureDataHandle handle;
	 Texture(TextureDataHandle h = TextureDataHandle()) : handle(h) {};
};