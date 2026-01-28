#include "texture.h"

TextureData::TextureData()
{
	pixels = nullptr;
	width = 0;
	height = 0;
	depth = 0;
	bpp = 0;
	image = Image::Create();
}