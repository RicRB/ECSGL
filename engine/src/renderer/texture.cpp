#include "texture.h"

TextureData::TextureData()
{
	id = 0;
	pixels = nullptr;
	width = 0;
	height = 0;
	bpp = 0;
	image = TextureImage::Create();
}
