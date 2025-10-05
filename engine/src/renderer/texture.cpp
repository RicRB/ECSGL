#include "texture.h"

TextureData::TextureData()
{
	id = "";
	pixels = nullptr;
	width = 0;
	height = 0;
	bpp = 0;
	image = TextureImage::Create();
}

Texture::Texture(TextureID id)
{
	this->id = id;
}