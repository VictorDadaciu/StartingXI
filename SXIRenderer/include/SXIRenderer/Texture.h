#pragma once

#include <string>
#include "SXICore/Types.h"

namespace sxi
{
	class Texture
	{
	public:
		Texture(const std::string&);
		Texture(Texture&) = delete;
		Texture(Texture&&) = default;

		Texture& operator=(Texture&) = delete;
		Texture& operator=(Texture&&) = default;

		~Texture();

		inline unsigned char* raw() const { return data; }
		inline u64 size() const { return m_size; }
		inline int width() const { return m_width; }
		inline int height() const { return m_height; }
		inline int channels() const { return m_channels; }
		inline u32 mipLevels() const { return m_mipLevels; }

	private:
		unsigned char* data{};
		u64 m_size;
		int m_width{};
		int m_height{};
		int m_channels{};
		u32 m_mipLevels{};
	};
}

