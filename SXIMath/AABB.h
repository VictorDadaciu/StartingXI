#pragma once

#include "Vec.h"

namespace sxi
{
	struct AABB
	{
		AABB() = default;
		AABB(float, float, float, float);
		AABB(const glm::vec2&, const glm::vec2&);
		AABB(const AABB&);
		AABB(AABB&&);

		inline static AABB invalid() { return AABB(SXI_VEC2_MAX, SXI_VEC2_MIN); }

		inline const glm::vec2 size() const { return botRight - topLeft; }
		inline const glm::vec2 center() const { return topLeft + 0.5f * size(); }

		inline float area() const { return (botRight.x - topLeft.x) * (botRight.y - topLeft.y); }
		inline float margin() const { return botRight.x - topLeft.x + botRight.y - topLeft.y; }
		inline float perimeter() const { return 2 * margin(); }

		inline float height() const { return size().x; };
		inline float width() const { return size().y; };

		inline bool inside(float x, float y) const { return topLeft.x <= x && x <= botRight.x && topLeft.y <= y && y <= botRight.y; }
		inline bool inside(const glm::vec2& p) const { return topLeft.x <= p.x && p.x <= botRight.x && topLeft.y <= p.y && p.y <= botRight.y; }

		inline float sqrDistance(const glm::vec2& point) const
		{
			float dx = fmaxf(point.x - botRight.x, fmaxf(topLeft.x - point.x, 0));
			float dy = fmaxf(point.y - botRight.y, fmaxf(topLeft.y - point.y, 0));
			return dx * dx + dy * dy;
		}
		inline float distance(const glm::vec2& point) const
		{
			return sqrt(sqrDistance(point));
		}

		bool intersects(const AABB&) const;
		float overlapArea(const AABB&) const;
		static AABB combine(const AABB&, const AABB&);

		inline void operator=(const AABB& other)
		{
			topLeft = other.topLeft;
			botRight = other.botRight;
		}

		inline void operator=(AABB&& other)
		{
			topLeft = other.topLeft;
			botRight = other.botRight;
		}

		inline bool operator==(const AABB& other) const { return topLeft == other.topLeft && botRight == other.botRight; }
		inline bool operator==(AABB&& other) const { return topLeft == other.topLeft && botRight == other.botRight; }

		glm::vec2 topLeft = SXI_VEC2_ZERO;
		glm::vec2 botRight = SXI_VEC2_ZERO;
	};
}

