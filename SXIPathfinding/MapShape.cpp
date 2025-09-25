#include "MapShape.h"

#include "CDT.h"
#include "Line.h"
#include <unordered_set>

namespace sxi
{
	MapShape::MapShape(const std::vector<QuarterEdge*>& edges) 
		: edges(edges)
	{
		const std::unordered_set<QuarterEdge*> outerEdges(edges.begin(), edges.end());
		std::unordered_set<QuarterEdge*> innerEdgesAlreadySeen;
		// construct internal MapObstacles
		for (const QuarterEdge* edge : edges)
		{
			QuarterEdge* lnext = edge->sym->prevOn();
			if (outerEdges.find(lnext) == outerEdges.end() && innerEdgesAlreadySeen.find(lnext) == innerEdgesAlreadySeen.end())
			{
				internals.emplace_back(lnext);
				innerEdgesAlreadySeen.insert(lnext);
				{
					QuarterEdge* ptr = lnext->sym->prevOn();
					while (ptr != lnext)
					{
						if (outerEdges.find(ptr) == outerEdges.end())
							innerEdgesAlreadySeen.insert(ptr);
						ptr = ptr->sym->prevOn();
					}
				}
				lnext = lnext->sym;
				if (innerEdgesAlreadySeen.find(lnext) == innerEdgesAlreadySeen.end())
				{
					internals.emplace_back(lnext);
					innerEdgesAlreadySeen.insert(lnext);
					QuarterEdge* ptr = lnext->sym->prevOn();
					while (ptr != lnext)
					{
						if (outerEdges.find(ptr) == outerEdges.end())
							innerEdgesAlreadySeen.insert(ptr);
						ptr = ptr->sym->prevOn();
					}
				}
			}
		}
		if (internals.empty())
			internals.push_back(edges[0]);
	}

	AABB MapShape::generateBoundingBox() const
	{
		AABB aabb(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
		for (const QuarterEdge* edge : edges)
		{
			aabb.topLeft.x = std::min(edge->data->v.x, aabb.topLeft.x);
			aabb.topLeft.y = std::min(edge->data->v.y, aabb.topLeft.y);
			aabb.botRight.x = std::max(edge->data->v.x, aabb.botRight.x);
			aabb.botRight.y = std::max(edge->data->v.y, aabb.botRight.y);
		}
		return aabb;
	}

	bool MapShape::inside(const glm::vec2& point) const
	{
		for (QuarterEdge* internal : internals)
		{
			bool isInside = true;
			QuarterEdge* ptr = internal;
			do
			{
				if (vec::sign(ptr->data->v, ptr->sym->data->v, point) > 0)
				{
					isInside = false;
					break;
				}
				ptr = ptr->sym->prevOn();
			} while (ptr != internal);
			if (isInside)
				return true;
		}
		return false;
	}

	// Return the closest edge to the closest point outside of the shape
	QuarterEdge* MapShape::closestEdgeForPointInside(const glm::vec2& point, glm::vec2& newPoint) const
	{
		QuarterEdge* closestEdge = nullptr;
		float minSqrDist = std::numeric_limits<float>::max();
		newPoint = SXI_VEC2_MAX;
		for (const QuarterEdge* edge : edges)
		{
			Line line(edge->data->v, edge->sym->data->v);
			float sqrDist;
			glm::vec2 p = line.closestNewPointOutsideAndSqrDist(point, sqrDist);
			if (sqrDist < minSqrDist)
			{
				minSqrDist = sqrDist;
				newPoint = p;
				closestEdge = edge->sym;
			}
		}
		return closestEdge;
	}

	QuarterEdge* MapShape::closestEdgeForPointOutside(const glm::vec2& point) const
	{
		QuarterEdge* closestEdge = nullptr;
		float minSqrDist = std::numeric_limits<float>::max();
		for (const QuarterEdge* edge : edges)
		{
			Line line(edge->data->v, edge->sym->data->v);
			float sqrDist = line.sqrDistToClosestPoint(point);
			if (sqrDist < minSqrDist)
			{
				minSqrDist = sqrDist;
				closestEdge = edge->sym;
			}
		}
		return closestEdge;
	}
}