#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace sxi
{
    Texture::Texture(const std::string& path)
	{
        data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        size = SXI_TO_U64(width) * height * 4;

        if (!data)
            throw std::exception("Failed to load texture image");
	}

    Texture::~Texture()
    {
        stbi_image_free(data);
    }
}