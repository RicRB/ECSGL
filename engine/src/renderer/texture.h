#pragma once
#include <string>
#include "image.h"
#include "memory/memory_pool.hpp"

enum class TextureUsage {
	Read,
	ReadWrite
};


struct TextureCreateInfo {
	TextureUsage usage;
	int width, height, depth;
};

struct TextureData
{
	TextureData();
	unsigned char* pixels;
	int width, height, depth, bpp;
	TextureUsage usage;
	std::shared_ptr<Image> image;
};

using TextureDataHandle = MemoryHandle;

struct Texture
{
	 TextureDataHandle handle;
	 Texture(TextureDataHandle h = TextureDataHandle()) : handle(h) {};
};