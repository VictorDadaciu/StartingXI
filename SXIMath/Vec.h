#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/common.hpp>

#define SXI_VEC2_ZERO glm::vec2(0.f)
#define SXI_VEC2_MIN  glm::vec2(std::numeric_limits<float>::min())
#define SXI_VEC2_MAX  glm::vec2(std::numeric_limits<float>::max())

#define SXI_VEC3_ZERO glm::vec3(0.f)
#define SXI_VEC3_MIN  glm::vec3(std::numeric_limits<float>::min())
#define SXI_VEC3_MAX  glm::vec3(std::numeric_limits<float>::max())

namespace sxi 
{
	namespace vec
	{
		inline float sqrLength(const glm::vec2& vec) { return glm::dot(vec, vec); }
		inline float sqrLength(const glm::vec3& vec) { return glm::dot(vec, vec); }

		inline float cross(const glm::vec2& a, const glm::vec2& b) { return a.x * b.y - b.x * a.y; }

		inline float sign(const glm::vec2& from, const glm::vec2& to, const glm::vec2& point)
		{
			return (to.x - from.x) * (point.y - from.y) - (to.y - from.y) * (point.x - from.x);
		}
	}
}

template <>
struct std::hash<glm::vec2>
{
	std::size_t operator()(const glm::vec2& v) const
	{
		std::size_t h1 = std::hash<float>{}(v.x);
		std::size_t h2 = std::hash<float>{}(v.y);
		return h1 ^ (h2 << 1);
	}
};

template <>
struct std::hash<glm::vec3>
{
	std::size_t operator()(const glm::vec3& v) const
	{
		std::size_t h1 = std::hash<float>{}(v.x);
		std::size_t h2 = std::hash<float>{}(v.y);
		std::size_t h3 = std::hash<float>{}(v.z);
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

