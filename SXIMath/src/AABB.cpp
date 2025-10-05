#include "AABB.h"

#include <cmath>

namespace sxi
{
	AABB::AABB(float x1, float y1, float x2, float y2)
	{
		this->topLeft = glm::vec2(x1, y1);
		this->botRight = glm::vec2(x2, y2);
	}

	AABB::AABB(const glm::vec2& topLeft, const glm::vec2& botRight)
	{
		this->topLeft = topLeft;
		this->botRight = botRight;
	}

	AABB::AABB(const AABB& other)
	{
		topLeft = other.topLeft;
		botRight = other.botRight;
	}

	AABB::AABB(AABB&& other)
	{
		topLeft = other.topLeft;
		botRight = other.botRight;
	}

	bool AABB::intersects(const AABB& other) const
	{
		glm::vec2 halfS = size() * 0.5f;
		glm::vec2 oHalfS = other.size() * 0.5f;
		return fabsf((topLeft.x + halfS.x) - (other.topLeft.x + oHalfS.x)) <= (halfS.x + oHalfS.x) &&
			fabsf((topLeft.y + halfS.y) - (other.topLeft.y + oHalfS.y)) <= (halfS.y + oHalfS.y);
	}

	float AABB::overlapArea(const AABB& other) const
	{
		float top = std::fmaxf(topLeft.x, other.topLeft.x);
		float left = std::fmaxf(topLeft.y, other.topLeft.y);
		float bot = std::fminf(botRight.x, other.botRight.x);
		float right = std::fminf(botRight.y, other.botRight.y);
		if (right < left || bot < top)
			return 0;
		return (right - left) * (bot - top);
	}

	AABB AABB::combine(const AABB& a, const AABB& b)
	{
		return AABB(std::fminf(a.topLeft.x, b.topLeft.x), std::fminf(a.topLeft.y, b.topLeft.y), std::fmaxf(a.botRight.x, b.botRight.x), std::fmaxf(a.botRight.y, b.botRight.y));
	}
}