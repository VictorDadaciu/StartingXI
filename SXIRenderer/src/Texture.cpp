#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cmath>
#include <stdexcept>

namespace sxi
{
    Texture::Texture(const std::string& path)
	{
        data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
        m_size = SXI_TO_U64(m_width) * m_height * 4;
        m_mipLevels = SXI_TO_U32(std::floor(std::log2(std::max(m_width, m_height)))) + 1;

        if (!data)
            throw std::runtime_error("Failed to load texture image");
	}

    Texture::~Texture()
    {
        stbi_image_free(data);
    }
}