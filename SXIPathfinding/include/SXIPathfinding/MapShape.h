#pragma once

#include "RStarTree.h"

#include <vector>

#include "SXIMath/Vec.h"

namespace sxi
{
	struct QuarterEdge;
	struct MapPoint
	{
		MapPoint(const glm::vec2& vec) : v(vec) {}

		glm::vec2 v = SXI_VEC2_MAX;
		QuarterEdge* start = nullptr;
	};

	struct RSTLeaf;
	class MapShape
	{
	public:
		MapShape() = default;
		MapShape(const std::vector<QuarterEdge*>&);

		bool inside(const glm::vec2&) const;
		QuarterEdge* closestEdgeForPointOutside(const glm::vec2&) const;
		QuarterEdge* closestEdgeForPointInside(const glm::vec2&, glm::vec2&) const;

		AABB generateBoundingBox() const;

	private:
		std::vector<QuarterEdge*> internals{};
		std::vector<QuarterEdge*> edges{};
		RSTLeaf* leaf = nullptr;

		friend class Map;
		friend class CDT;
	};
}

