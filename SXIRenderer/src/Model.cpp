#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <stdexcept>
#include <unordered_map>

#include "Texture.h"

namespace sxi
{
	Model::Model(const std::string& path, Texture* tex) : tex(tex)
	{
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
            throw std::runtime_error(err.c_str());

        std::unordered_map<Vertex, u32> uniqueVerts{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (index.texcoord_index == -1)
                {
                    vertex.uv = SXI_VEC2_ZERO;
                }
                else
                {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                vertex.col = {1.0f, 1.0f, 1.0f};

                if (uniqueVerts.count(vertex) == 0)
                {
                    uniqueVerts[vertex] = SXI_TO_U32(verts.size());
                    verts.push_back(std::move(vertex));
                }

                indis.push_back(uniqueVerts[vertex]);
            }
        }
	}
}