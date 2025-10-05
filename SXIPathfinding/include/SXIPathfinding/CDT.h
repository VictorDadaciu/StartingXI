#pragma once

#include "MapShape.h"

#include <memory>
#include <vector>
#include <unordered_set>

#include "SXIMath/Vec.h"

namespace sxi
{
	struct QuarterEdge
	{
		MapPoint* data = nullptr;
		QuarterEdge* next = nullptr;
		QuarterEdge* prev = nullptr;
		QuarterEdge* sym = nullptr;
		bool on = false;
		bool constrained = false;

		QuarterEdge* prevOn() const;
	};

	class PointMap;
	class CDT
	{
	public:
		CDT() = default;
		CDT(float, float, float, float);
		~CDT();

		MapShape insertShape(const std::vector<glm::vec2>&, QuarterEdge*);
		QuarterEdge* find(float, float, QuarterEdge*) const;
		QuarterEdge* find(const glm::vec2&, QuarterEdge*) const;
		void collect(std::vector<glm::vec2>&, std::vector<bool>&, std::vector<int>&, bool) const;
		std::vector<glm::vec2> removeShape(const MapShape&);

		glm::vec2 mapTopLeft;
		glm::vec2 mapBotRight;

	private:
		QuarterEdge* insert(MapPoint*, QuarterEdge*, std::unordered_set<QuarterEdge*>&);
		void setOn(QuarterEdge*, bool) const;
		void findNewOnForPoints(QuarterEdge*) const;
		QuarterEdge* findEdge(float, float, QuarterEdge*) const;
		QuarterEdge* findEdge(const glm::vec2&, QuarterEdge*) const;
		bool passesBoundaryRules(QuarterEdge*, QuarterEdge*) const;
		QuarterEdge* makeQuadEdge(MapPoint*, MapPoint*) const;
		QuarterEdge* makeTriangle(MapPoint*, MapPoint*, MapPoint*) const;
		QuarterEdge* connect(QuarterEdge*, QuarterEdge*) const;
		QuarterEdge* insertPoint(QuarterEdge*, MapPoint*) const;
		void flip(QuarterEdge*) const;
		bool convex(QuarterEdge*) const;
		bool convexOnlyOn(QuarterEdge*) const;
		void collect(std::vector<MapPoint*>&, std::vector<QuarterEdge*>&) const;
		QuarterEdge* connectionExists(MapPoint*, MapPoint*) const;
		QuarterEdge* forceConnect(MapPoint*, MapPoint*, std::unordered_set<QuarterEdge*>&) const;
		std::vector<glm::vec2> removePoint(MapPoint*);

		inline bool isBoundaryPoint(const glm::vec2& v) const
		{
			return v.x < mapTopLeft.x - 5 || v.x > mapBotRight.x + 5 || v.y < mapTopLeft.y - 5 || v.y > mapBotRight.y + 5;
		}

		inline bool withBoundary(QuarterEdge* edge) const
		{
			return isBoundaryPoint(edge->data->v) || isBoundaryPoint(edge->sym->data->v);
		}

		inline bool inTriangle(QuarterEdge* edge, MapPoint* target) const
		{
			return edge->data == target || edge->sym->data == target || edge->next->sym->data == target;
		}

		inline void swapNexts(QuarterEdge* a, QuarterEdge* b) const
		{
			QuarterEdge* anext = a->next;
			a->next = b->next;
			b->next = anext;
		}

		inline void swapPrevs(QuarterEdge* a, QuarterEdge* b) const
		{
			QuarterEdge* aprev = a->prev;
			a->prev = b->prev;
			b->prev = aprev;
		}

		inline void splice(QuarterEdge* left, QuarterEdge* n) const
		{
			QuarterEdge* right = left->next;
			swapNexts(left, n);
			swapPrevs(n, right);
		}

		inline void desplice(QuarterEdge* left, QuarterEdge* n) const
		{
			QuarterEdge* right = n->next;
			swapNexts(left, n);
			swapPrevs(n, right);
		}

		inline bool insideCircumspectCircle(MapPoint* a, MapPoint* b, MapPoint* c, MapPoint* d) const
		{
			float ax = a->v.x - d->v.x;
			float bx = b->v.x - d->v.x;
			float cx = c->v.x - d->v.x;
			float ay = a->v.y - d->v.y;
			float by = b->v.y - d->v.y;
			float cy = c->v.y - d->v.y;
			float aSqr = ax * ax + ay * ay;
			float bSqr = bx * bx + by * by;
			float cSqr = cx * cx + cy * cy;
			return aSqr * (bx * cy - cx * by) - bSqr * (ax * cy - cx * ay) + cSqr * (ax * by - bx * ay) < 0;
		}

		QuarterEdge* fallback;
		const float CDT_BUFFER = 50;
		const float MAP_BUFFER = 5;
	};
}
