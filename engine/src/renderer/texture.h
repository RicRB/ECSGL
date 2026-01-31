#pragma once
#include <string>
#include "image.h"
#include "memory/memory_pool.hpp"

enum class TextureUsage {
	Read,
	ReadWrite
};

enum class TextureFormat {
	R16G16_SFLOAT,
	R16G16_SNORM,
	R16G16_UNORM,
	R8G8B8A8_UNORM,
	R16G16B16A16_UNORM,
	R32G32B32A32_SFLOAT,
};



struct TextureCreateInfo {
	TextureUsage usage = TextureUsage::ReadWrite;
	TextureFormat format = TextureFormat::R8G8B8A8_UNORM;
	int width, height, depth = 1;
};

struct TextureData
{
	TextureData();
	unsigned char* pixels;
	int width, height, depth, bpp;
	TextureUsage usage;
	TextureFormat format;
	std::shared_ptr<Image> image;
};

using TextureDataHandle = MemoryHandle;

struct Texture
{
	 TextureDataHandle handle;
	 Texture(TextureDataHandle h = TextureDataHandle()) : handle(h) {};

	 static size_t getFormatPixelByteSize(TextureFormat format)
	 {
		 switch (format)
		 {
			 case TextureFormat::R16G16_SFLOAT: return 4;
			 case TextureFormat::R16G16_SNORM: return 4;
			 case TextureFormat::R16G16_UNORM: return 4;
			 case TextureFormat::R8G8B8A8_UNORM: return 4;
			 case TextureFormat::R16G16B16A16_UNORM: return 8;
			 case TextureFormat::R32G32B32A32_SFLOAT: return 16;
		 }
	 }
};