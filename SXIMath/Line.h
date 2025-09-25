#pragma once

#include "Vec.h"

namespace sxi 
{
	struct Line 
	{
		Line(const glm::vec2&, const glm::vec2&);

		inline bool below(const glm::vec2& point) const { return (end.x - start.x) * (point.y - start.y) - (end.y - start.y) * (point.x - start.x) < 0; }
		float sqrDistToClosestPoint(const glm::vec2& point) const;
		glm::vec2 closestNewPointOutsideAndSqrDist(const glm::vec2&, float&) const;
		inline glm::vec2 mirror(const glm::vec2& point) const
		{
			glm::vec2 ab = end - start;
			return point + 2.f * (start + ab * (glm::dot(ab, point - start) / vec::sqrLength(ab)) - point);
		}

		bool intersects(const Line&) const;
		inline float sqrLength() const { return vec::sqrLength(end - start); }

		glm::vec2 start = SXI_VEC2_ZERO;
		glm::vec2 end = SXI_VEC2_ZERO;
	private:

		bool intersectsOneWay(const Line&) const;
	};

	struct Ray 
	{
		Ray(const glm::vec2 &, const glm::vec2 &);

		glm::vec2 origin = SXI_VEC2_ZERO;
		glm::vec2 dir = SXI_VEC2_ZERO;

		inline bool below(const glm::vec2 &point) const 
		{
			return dir.x * (point.y - origin.y) - dir.y * (point.x - origin.x) < 0;
		}

		inline glm::vec2 mirror(const glm::vec2 &point) const 
		{
			return point + 2.f * (origin + dir * (glm::dot(dir, point - origin) / vec::sqrLength(dir)) - point);
		}

		bool between(const Ray &, const Ray &) const;

		bool intersects(const Line &, glm::vec2 &) const;
		inline bool operator==(const Ray& other) const { return origin == other.origin && dir == other.dir; }
	};
}
