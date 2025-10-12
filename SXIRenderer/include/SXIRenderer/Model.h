#pragma once

#include "Texture.h"

#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>
#include "SXIMath/Vec.h"
#include "SXICore/Types.h"

namespace sxi::renderer
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 norm;
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
			attributeDescs[1].offset = offsetof(Vertex, norm);
			attributeDescs[2].binding = 0;
			attributeDescs[2].location = 2;
			attributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescs[2].offset = offsetof(Vertex, uv);
			return attributeDescs;
		}

		inline bool operator==(const Vertex& other) const { return pos == other.pos && norm == other.norm && uv == other.uv; }
	};

	struct Model
	{
		std::vector<Vertex> verts{};
		VkDeviceSize vertexBufferOffset{};
		std::vector<u32> indices{};
		VkDeviceSize indexBufferOffset{};

		Model(const std::string&);

		Model(Model&) = delete;
		Model(Model&&) = default;

		Model& operator=(Model&) = delete;
		Model& operator=(Model&&) = default;
	};

    extern Model* model;
}

namespace std
{
	template<> struct hash<sxi::renderer::Vertex>
	{
		size_t operator()(const sxi::renderer::Vertex& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					 (hash<glm::vec3>()(vertex.norm) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}

