#pragma once

#include <string>
#include "Types.h"

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
		inline u64 getSize() const { return size; }
		inline int getWidth() const { return width; }
		inline int getHeight() const { return height; }
		inline int getChannels() const { return channels; }

	private:
		unsigned char* data{};
		u64 size;
		int width{};
		int height{};
		int channels{};
	};
}

