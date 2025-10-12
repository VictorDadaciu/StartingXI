#include "Model.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>

#include "SXICore/Exception.h"

namespace sxi::renderer
{
    Model* model{};

	Model::Model(const std::string& path)
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

                vertex.norm = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
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

                if (uniqueVerts.count(vertex) == 0)
                {
                    uniqueVerts[vertex] = SXI_TO_U32(verts.size());
                    verts.push_back(std::move(vertex));
                }

                indices.push_back(uniqueVerts[vertex]);
            }
        }
	}
}