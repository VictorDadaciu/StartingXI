#pragma once

#include "Texture.h"

#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>
#include "Vec.h"
#include "Types.h"

namespace sxi
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 col;
		glm::vec2 uv;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = 0;
			bindingDesc.stride = sizeof(Vertex);
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDesc;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescs{};
			attributeDescs[0].binding = 0;
			attributeDescs[0].location = 0;
			attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescs[0].offset = offsetof(Vertex, pos);
			attributeDescs[1].binding = 0;
			attributeDescs[1].location = 1;
			attributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescs[1].offset = offsetof(Vertex, col);
			attributeDescs[2].binding = 0;
			attributeDescs[2].location = 2;
			attributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescs[2].offset = offsetof(Vertex, uv);
			return attributeDescs;
		}

		inline bool operator==(const Vertex& other) const { return pos == other.pos && col == other.col && uv == other.uv; }
	};

	class Model
	{
	public:
		Model(const std::string&, Texture*);

		Model(Model&) = delete;
		Model(Model&&) = default;

		Model& operator=(Model&) = delete;
		Model& operator=(Model&&) = default;

		inline const Texture* texture() const { return tex; }
		inline const std::vector<Vertex>& vertices() const { return verts; }
		inline const std::vector<u32>& indices() const { return indis; }

	private:
		std::vector<Vertex> verts{};
		std::vector<u32> indis{};
		Texture* tex{};
	};
}

namespace std
{
	template<> struct hash<sxi::Vertex>
	{
		size_t operator()(const sxi::Vertex& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					 (hash<glm::vec3>()(vertex.col) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}

