#include "CDT.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <set>
#include "MapShape.h"
#include "Line.h"

namespace sxi
{
	CDT::CDT(float x1, float y1, float x2, float y2) : mapTopLeft(x1, y1), mapBotRight(x2, y2)
	{
		MapPoint* topLeft = new MapPoint(glm::vec2(x1 - CDT_BUFFER, y1 - CDT_BUFFER));
		MapPoint* topRight = new MapPoint(glm::vec2(x1 - CDT_BUFFER, y1 + 2 * (y2 - y1) + 3 * CDT_BUFFER));
		MapPoint* botLeft = new MapPoint(glm::vec2(x1 + 2 * (x2 - x1) + 3 * CDT_BUFFER, y1 - CDT_BUFFER));
		fallback = makeTriangle(topLeft, topRight, botLeft);
		MapPoint* mapTopLeft = new MapPoint(glm::vec2(x1 - MAP_BUFFER, y1 - MAP_BUFFER));
		MapPoint* mapTopRight = new MapPoint(glm::vec2(x1 - MAP_BUFFER, y2 + MAP_BUFFER));
		MapPoint* mapBotRight = new MapPoint(glm::vec2(x2 + MAP_BUFFER, y2 + MAP_BUFFER));
		MapPoint* mapBotLeft = new MapPoint(glm::vec2(x2 + MAP_BUFFER, y1 - MAP_BUFFER));
		std::unordered_set<QuarterEdge*> toCull;
		insert(mapTopLeft, nullptr, toCull);
		insert(mapTopRight, nullptr, toCull);
		insert(mapBotRight, nullptr, toCull);
		insert(mapBotLeft, nullptr, toCull);
		fallback = forceConnect(mapTopLeft, mapTopRight, toCull);
		forceConnect(mapTopRight, mapBotRight, toCull);
		forceConnect(mapBotRight, mapBotLeft, toCull);
		forceConnect(mapBotLeft, mapTopLeft, toCull);
		for (QuarterEdge* edge : toCull)
			if (!edge->constrained && edge->on)
				setOn(edge, !convexOnlyOn(edge));
	}

	CDT::~CDT()
	{
		if (!fallback)
			return;

		std::vector<MapPoint*> points;
		std::vector<QuarterEdge*> edges;
		collect(points, edges);
		for (int i = 0; i < points.size(); i++)
			delete points[i];
		for (int i = 0; i < edges.size(); i++)
			delete edges[i];
	}

	bool CDT::passesBoundaryRules(QuarterEdge* edge, QuarterEdge* opposite) const
	{
		// boundary edge
		if (isBoundaryPoint(edge->data->v) && isBoundaryPoint(edge->sym->data->v))
			return false;

		// MapPoint is opposite a boundary MapPoint
		if (isBoundaryPoint(opposite->data->v))
			return false;

		// flipping would cause inside out triangle
		if (withBoundary(edge) && !convex(edge))
			return false;

		// all good
		return true;
	}

	inline static float radiusOfTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		float AB = glm::length(b - a);
		float BC = glm::length(c - b);
		float CA = glm::length(a - c);
		return AB * BC * CA / sqrtf((AB + BC + CA) * (AB + BC - CA) * (BC + CA - AB) * (CA + AB - BC));
	}

	std::vector<glm::vec2> CDT::removePoint(MapPoint* point)
	{
		// find enclosing polygon and delete point all edges inside
		std::vector<QuarterEdge*> polygon;
		std::vector<float> radii;
		{
			QuarterEdge* polyStartEdge = nullptr;
			std::unordered_set<QuarterEdge*> edgesToDelete;
			QuarterEdge* ptr = point->start;
			do 
			{
				edgesToDelete.insert(ptr);
				QuarterEdge* inwards = ptr->sym;
				edgesToDelete.insert(inwards);
				if (!polyStartEdge)
					polyStartEdge = inwards->prev;
				if (!isBoundaryPoint(inwards->data->v) && inwards->data->start == inwards)
					inwards->data->start = inwards->prevOn();
				desplice(inwards->prev, inwards);
				ptr = ptr->next;
			} while (ptr != point->start);
			// delete everything collected
			{
				delete point;
				std::vector<QuarterEdge*> edgesVec(edgesToDelete.begin(), edgesToDelete.end());
				for (int i = 0; i < edgesVec.size(); i++)
					delete edgesVec[i];
			}
			// get the enclosing polygon
			fallback = polyStartEdge;
			ptr = polyStartEdge;
			do
			{
				polygon.push_back(ptr);
				setOn(ptr, !withBoundary(ptr));
				ptr = ptr->sym->prev;
			} while (ptr != polyStartEdge);
			// calculate radii of adjacent vertices
			radii.resize(polygon.size());
			int left = polygon.size() - 1;
			glm::vec2 a = polygon[0]->data->v - polygon[left]->data->v;
			for (int i = 0; i < polygon.size(); i++)
			{
				int right = (i + 1) % polygon.size();
				glm::vec2 b = polygon[right]->data->v - polygon[i]->data->v;
				radii[i] = std::numeric_limits<float>::max();
				if (glm::cross(b, a) > 0)
					radii[i] = radiusOfTriangle(polygon[left]->data->v, polygon[i]->data->v, polygon[right]->data->v);
				left = i;
				a = b;
			}
		}
		std::unordered_set<QuarterEdge*> toCull(polygon.begin(), polygon.end());
		// make polygon smaller by adding edges between vertices
		while (polygon.size() > 3)
		{
			// find smallest circle
			int removalIndex = 0;
			float minRadius = radii[0];
			for (int i = 1; i < polygon.size(); i++)
			{
				if (radii[i] < minRadius)
				{
					removalIndex = i;
					minRadius = radii[i];
				}
			}
			// smallest circle found, make new edge, splice it in
			int left = (removalIndex - 1 + polygon.size()) % polygon.size();
			int right = (removalIndex + 1) % polygon.size();
			QuarterEdge* newEdge = makeQuadEdge(polygon[left]->data, polygon[right]->data);
			toCull.insert(newEdge);
			splice(polygon[left], newEdge);
			splice(polygon[right], newEdge->sym);
			// update polygon and radii list
			polygon[left] = newEdge;
			polygon.erase(polygon.begin() + removalIndex);
			radii.erase(radii.begin() + removalIndex);
			if (polygon.size() <= 3)
				break;
			int midLeft = std::min(left, (int)polygon.size() - 1);
			int midRight = (midLeft + 1) % polygon.size();
			left = (midLeft - 1 + polygon.size()) % polygon.size();
			right = (midRight + 1) % polygon.size();
			glm::vec2 a = polygon[midLeft]->data->v - polygon[left]->data->v;
			glm::vec2 b = polygon[midRight]->data->v - polygon[midLeft]->data->v;
			glm::vec2 c = polygon[right]->data->v - polygon[midRight]->data->v;
			radii[midLeft] = std::numeric_limits<float>::max();
			radii[midRight] = std::numeric_limits<float>::max();
			if (glm::cross(b, a) > 0)
				radii[midLeft] = radiusOfTriangle(polygon[left]->data->v, polygon[midLeft]->data->v, polygon[midRight]->data->v);
			if (glm::cross(c, b) > 0)
				radii[midRight] = radiusOfTriangle(polygon[midLeft]->data->v, polygon[midRight]->data->v, polygon[right]->data->v);
		}
		// cull all affected edges
		for (QuarterEdge* edge : toCull)
			if (!edge->constrained && edge->on)
				setOn(edge, !convexOnlyOn(edge));
		std::vector<glm::vec2> retVal(polygon.size());
		std::transform(polygon.cbegin(), polygon.cend(), retVal.begin(), [](QuarterEdge* edge) { return edge->data->v; });
		return retVal;
	}

	std::vector<glm::vec2> CDT::removeShape(const MapShape& shape)
	{
		std::vector<MapPoint*> pointsToDelete(shape.edges.size());
		for (int i = 0; i < pointsToDelete.size(); i++)
			pointsToDelete[i] = shape.edges[i]->data;
		for (int i = 0; i < pointsToDelete.size() - 1; i++)
			removePoint(pointsToDelete[i]);
		return removePoint(pointsToDelete[pointsToDelete.size() - 1]);
	}

	QuarterEdge* CDT::forceConnect(MapPoint* start, MapPoint* end, std::unordered_set<QuarterEdge*>& toCull) const
	{
		if (QuarterEdge* edge = connectionExists(start, end))
		{
			edge->constrained = true;
			edge->sym->constrained = true;
			return edge;
		}

		// look for first intersection
		std::queue<QuarterEdge*> intersections;
		QuarterEdge* ptr = start->start;
		Line startEnd(start->v, end->v);
		QuarterEdge* edge = nullptr;
		do
		{
			edge = ptr->sym->prev;
			Line edgeLine(edge->data->v, edge->sym->data->v);
			if (startEnd.intersects(edgeLine))
			{
				edge = edge->sym;
				intersections.push(edge);
				setOn(ptr, !withBoundary(ptr));
				if (ptr->on)
					toCull.insert(ptr);
				setOn(ptr->next, !withBoundary(ptr->next));
				if (ptr->next->on)
					toCull.insert(ptr->next);
				break;
			}
			ptr = ptr->next;
		} while (ptr != start->start);

		// look for all intersections
		Ray endRay(start->v, end->v - start->v);
		while (!inTriangle(edge, end))
		{
			{
				ptr = edge;
				QuarterEdge* next = ptr->next;
				Ray ptrRay(start->v, ptr->sym->data->v - start->v);
				Ray nextRay(start->v, next->sym->data->v - start->v);
				if (endRay.between(ptrRay, nextRay))
				{
					setOn(ptr, !withBoundary(ptr));
					if (ptr->on)
						toCull.insert(ptr);
					setOn(next, !withBoundary(next));
					if (next->on)
						toCull.insert(next);
					edge = next->sym->next;
					if (edge->constrained)
						throw std::exception("Cannot intersect constrained edges");
					intersections.push(edge);
					continue;
				}
			}
			{
				ptr = edge->sym;
				QuarterEdge* prev = ptr->prev;
				Ray ptrRay(start->v, ptr->sym->data->v - start->v);
				Ray prevRay(start->v, prev->sym->data->v - start->v);
				if (endRay.between(ptrRay, prevRay))
				{
					setOn(ptr, !withBoundary(ptr));
					if (ptr->on)
						toCull.insert(ptr);
					setOn(prev, !withBoundary(prev));
					if (prev->on)
						toCull.insert(prev);
					edge = ptr->sym->next;
					if (edge->constrained)
						throw std::exception("Cannot intersect constrained edges");
					intersections.push(edge);
					continue;
				}
			}
		}
		ptr = edge;
		do
		{
			setOn(ptr, !withBoundary(ptr));
			if (ptr->on)
				toCull.insert(ptr);
			ptr = ptr->sym->prev;
		} while (ptr != edge);

		// loop through intersections
		std::vector<QuarterEdge*> newEdges;
		while (!intersections.empty())
		{
			edge = intersections.front();
			intersections.pop();
			if (convex(edge))
			{
				flip(edge);
				Ray dataRay(start->v, edge->data->v - start->v);
				Ray symRay(start->v, edge->sym->data->v - start->v);
				// intersection rules and edge-cases (pun not intended)
				if (edge->data == start || edge->sym->data == end || !endRay.between(dataRay, symRay))
				{
					newEdges.push_back(edge);
					continue;
				}
			}
			intersections.push(edge);
		}

		// loop through new edges until no more swaps happen
		QuarterEdge* retVal = nullptr;
		bool swapped = true;
		while (swapped)
		{
			swapped = false;
			for (QuarterEdge* edge : newEdges)
			{
				if (edge->data == start && edge->sym->data == end)
				{
					retVal = edge;
					continue;
				}
				QuarterEdge* opposite = edge->prev->sym->prev;
				if (!edge->constrained && 
					passesBoundaryRules(edge, opposite) && (withBoundary(edge) ||
					insideCircumspectCircle(ptr->data, ptr->sym->data, opposite->sym->data, opposite->data) ||
						insideCircumspectCircle(ptr->sym->data, opposite->data, opposite->sym->data, ptr->data)))
				{
					flip(edge);
					swapped = true;
				}
			}
		}
		if (!retVal)
			throw std::exception("Could not create a constrained edge");
		retVal->constrained = true;
		retVal->sym->constrained = true;
		return retVal;
	}

	QuarterEdge* CDT::connectionExists(MapPoint* start, MapPoint* end) const
	{
		QuarterEdge* ptr = start->start;
		do
		{
			if (ptr->sym->data == end)
				return ptr;
			ptr = ptr->prev;
		} while (ptr != start->start);
		return nullptr;
	}

	MapShape CDT::insertShape(const std::vector<glm::vec2>& coords, QuarterEdge* bestEdge)
	{ 
		std::vector<MapPoint*> points(coords.size());
		std::transform(coords.cbegin(), coords.cend(), points.begin(), [](const glm::vec2& e) { return new MapPoint(e); });
		std::unordered_set<QuarterEdge*> toCull;
		for (MapPoint* p : points)
			bestEdge = insert(p, bestEdge, toCull);
		int n = points.size();
		if (n-- <= 2)
			throw std::exception("Cannot make shape from 2 or fewer MapPoints");
		std::vector<QuarterEdge*> shapeEdges;
		for (int i = 0; i < n; i++)
			shapeEdges.push_back(forceConnect(points[i], points[i + 1], toCull));
		shapeEdges.push_back(forceConnect(points[n], points[0], toCull));
		fallback = shapeEdges[shapeEdges.size() - 1]->sym;

		// cull all affected edges
		for (QuarterEdge* edge : toCull)
			if (!edge->constrained && edge->on)
				setOn(edge, !convexOnlyOn(edge));
		return MapShape(shapeEdges);
	}

	QuarterEdge* CDT::insert(MapPoint* p, QuarterEdge* bestEdge, std::unordered_set<QuarterEdge*>& toCull)
	{
		QuarterEdge* current = insertPoint(findEdge(p->v, bestEdge), p)->sym;
		QuarterEdge* ptr = current;
		p->start = ptr;
		bool moved = false;
		do
		{
			QuarterEdge* edge = ptr->sym->prev;
			setOn(edge, !withBoundary(edge));
			QuarterEdge* opposite = edge->prev->sym->prev;
			// might need to flip, don't move
			if (!edge->constrained && 
				passesBoundaryRules(edge, opposite) && (withBoundary(edge) ||
				insideCircumspectCircle(ptr->data, ptr->sym->data, opposite->sym->data, opposite->data) ||
					insideCircumspectCircle(ptr->sym->data, opposite->data, opposite->sym->data, ptr->data)))
			{
				flip(edge);
				continue;
			}
			// cull newly updated vertices
			if (ptr->on)
				toCull.insert(ptr);
			if (edge->on)
				toCull.insert(edge);
			// all good
			ptr = ptr->next;
			moved = true;
		} while (!moved || ptr != current);
		return current;
	}

	QuarterEdge* QuarterEdge::prevOn() const
	{
		QuarterEdge* ptr = prev;
		while (!ptr->on && ptr != this)
			ptr = ptr->prev;
		return ptr;
	}

	bool CDT::convex(QuarterEdge* edge) const
	{
		QuarterEdge* start = edge->prev;
		QuarterEdge* ptr = start;
		glm::vec2 a = ptr->sym->data->v - ptr->data->v; 
		QuarterEdge* lNext = ptr->sym->prev;
		glm::vec2 b = lNext->sym->data->v - lNext->data->v;
		float prev = glm::cross(b, a);
		a = b;
		do
		{
			ptr = lNext;
			lNext = ptr->sym->prev;
			if (lNext == edge || lNext == edge->sym)
				lNext = lNext->prev;
			b = lNext->sym->data->v - lNext->data->v;
			float curr = glm::cross(b, a);
			a = b;
			if (curr != 0)
			{
				if (curr * prev < 0)
				{
					return false;
				}
				prev = curr;
			}
		} while (ptr != start);
		return true;
	}

	bool CDT::convexOnlyOn(QuarterEdge* edge) const
	{
		QuarterEdge* start = edge->prevOn();
		if (start == edge)
			return false;

		QuarterEdge* ptr = start;
		glm::vec2 a = ptr->sym->data->v - ptr->data->v;
		do
		{
			QuarterEdge* lNext = ptr->sym->prevOn();
			while (lNext == edge || lNext == edge->sym)
				lNext = lNext->prevOn();
			if (lNext == ptr->sym)
				return false;
			ptr = lNext;

			glm::vec2 b = ptr->sym->data->v - ptr->data->v;
			if (glm::cross(a, b) > 0)
				return false;
			a = b;
		} while (ptr != start);
		return true;
	}

	QuarterEdge* CDT::makeQuadEdge(MapPoint* start, MapPoint* end) const
	{
		QuarterEdge* startEnd = new QuarterEdge();
		QuarterEdge* endStart = new QuarterEdge();

		startEnd->data = start;
		endStart->data = end;

		startEnd->next = startEnd;
		startEnd->prev = startEnd;
		startEnd->sym = endStart;
		endStart->next = endStart;
		endStart->prev = endStart;
		endStart->sym = startEnd;

		bool off = withBoundary(startEnd);
		startEnd->on = !off;
		endStart->on = !off;

		return startEnd;
	}

	QuarterEdge* CDT::makeTriangle(MapPoint* a, MapPoint* b, MapPoint* c) const
	{
		QuarterEdge* ab = makeQuadEdge(a, b);
		QuarterEdge* bc = makeQuadEdge(b, c);
		QuarterEdge* ca = makeQuadEdge(c, a);
		splice(bc, ab->sym);
		splice(ca, bc->sym);
		splice(ab, ca->sym);
		a->start = ab;
		b->start = bc;
		c->start = ca;
		ab->constrained = true;
		ab->sym->constrained = true;
		bc->constrained = true;
		bc->sym->constrained = true;
		ca->constrained = true;
		ca->sym->constrained = true;
		return ab;
	}

	QuarterEdge* CDT::connect(QuarterEdge* a, QuarterEdge* b) const
	{
		QuarterEdge* newEdge = makeQuadEdge(a->sym->data, b->data);
		splice(a->sym->prev, newEdge);
		splice(b, newEdge->sym);
		return newEdge;
	}

	QuarterEdge* CDT::insertPoint(QuarterEdge* polygonEdge, MapPoint* MapPoint) const
	{
		QuarterEdge* firstSpoke = makeQuadEdge(polygonEdge->data, MapPoint);
		splice(polygonEdge, firstSpoke);
		QuarterEdge* spoke = firstSpoke;
		do {
			spoke = connect(polygonEdge, spoke->sym);
			polygonEdge = spoke->prev;
		} while (polygonEdge->sym->prev != firstSpoke);
		return firstSpoke;
	}

	void CDT::findNewOnForPoints(QuarterEdge* edge) const
	{
		if (!isBoundaryPoint(edge->data->v) && edge->data->start == edge)
			edge->data->start = edge->prevOn();
		if (!isBoundaryPoint(edge->sym->data->v) && edge->sym->data->start == edge->sym)
			edge->sym->data->start = edge->sym->prevOn();
	}

	void CDT::setOn(QuarterEdge* edge, bool on) const
	{
		edge->on = on;
		edge->sym->on = on;
		if (!on)
			findNewOnForPoints(edge);
	}

	void CDT::flip(QuarterEdge* edge) const
	{
		findNewOnForPoints(edge);
		QuarterEdge* a = edge->prev;
		QuarterEdge* b = edge->sym->prev;
		desplice(a, edge);
		desplice(b, edge->sym);
		splice(a->sym->prev, edge);
		splice(b->sym->prev, edge->sym);
		edge->data = a->sym->data;
		edge->sym->data = b->sym->data;
		setOn(edge, !withBoundary(edge));
	}

	void CDT::collect(std::vector<MapPoint*>& points, std::vector<QuarterEdge*>& edges) const
	{
		std::queue<QuarterEdge*> open;
		open.push(fallback);
		std::set<MapPoint*> pointsVisited;
		pointsVisited.insert(fallback->data);
		points.push_back(fallback->data);
		std::set<QuarterEdge*> edgesAdded;
		while (!open.empty())
		{
			QuarterEdge* ptr = open.front();
			open.pop();
			QuarterEdge* it = ptr;
			do
			{
				if (edgesAdded.find(it) == edgesAdded.end())
				{
					MapPoint* MapPoint = it->sym->data;
					if (pointsVisited.find(MapPoint) == pointsVisited.end())
					{
						pointsVisited.insert(MapPoint);
						points.push_back(MapPoint);
						open.push(it->sym->prev);
					}
					edgesAdded.insert(it);
					edgesAdded.insert(it->sym);
					edges.push_back(it);
					edges.push_back(it->sym);
				}
				it = it->next;
			} while (it != ptr);
		}
	}

	void CDT::collect(std::vector<glm::vec2>& points, std::vector<bool>& constrained, std::vector<int>& edges, bool onlyOn) const
	{
		std::queue<QuarterEdge*> open;
		open.push(fallback);
		points.push_back(fallback->data->v);
		std::unordered_map<MapPoint*, int> pointsVisited;
		pointsVisited[fallback->data] = 0;
		std::set<QuarterEdge*> edgesAdded;
		while (!open.empty())
		{
			QuarterEdge* ptr = open.front();
			open.pop();
			int ptrIndex = pointsVisited[ptr->data];
			QuarterEdge* it = ptr;
			do
			{
				if (edgesAdded.find(it) == edgesAdded.end() || edgesAdded.find(it->sym) == edgesAdded.end())
				{
					edgesAdded.insert(it);
					MapPoint* MapPoint = it->sym->data;
					if (pointsVisited.find(MapPoint) == pointsVisited.end())
					{
						pointsVisited[MapPoint] = points.size();
						points.push_back(MapPoint->v);
						open.push(it->sym->prev);
					}
					if (!onlyOn || it->on)
					{
						edges.push_back(ptrIndex);
						edges.push_back(pointsVisited[MapPoint]);
						constrained.push_back(it->constrained);
					}
				}
				it = it->next;
			} while (it != ptr);
		}
	}

	QuarterEdge* CDT::findEdge(float x, float y, QuarterEdge* bestEdge) const
	{
		return findEdge(glm::vec2(x, y), bestEdge);
	}

	QuarterEdge* CDT::findEdge(const glm::vec2& point, QuarterEdge* bestEdge) const
	{
		// find start edge
		QuarterEdge* current = bestEdge ? bestEdge : fallback;
		// find best edge from MapPoint
		QuarterEdge* ptr = current;
		do
		{
			if (glm::sign(ptr->data->v, ptr->sym->data->v, point) > 0)
				ptr = ptr->prev;
			else
				break;
		} while (ptr != current);
		current = ptr;
		ptr = ptr->sym->prev;
		bool moved = false;
		// find polyogn
		do
		{
			if (glm::sign(ptr->data->v, ptr->sym->data->v, point) > 0) // go to next 
			{
				current = ptr->prev;
				ptr = current;
				moved = false;
				continue;
			}
			ptr = ptr->sym->prev;
			moved = true;
		} while (!moved || ptr != current);
		return current;
	}

	QuarterEdge* CDT::find(float x, float y, QuarterEdge* bestEdge) const
	{
		return find(glm::vec2(x, y), bestEdge);
	}

	QuarterEdge* CDT::find(const glm::vec2& point, QuarterEdge* bestEdge) const
	{
		// find start edge
		QuarterEdge* current = bestEdge ? bestEdge : fallback;
		// find best edge from MapPoint
		QuarterEdge* ptr = current;
		do
		{
			if (glm::sign(ptr->data->v, ptr->sym->data->v, point) > 0)
				ptr = ptr->prevOn();
			else
				break;
		} while (ptr != current);
		current = ptr;
		ptr = ptr->sym->prevOn();
		bool moved = false;
		// find polyogn
		do
		{
			if (glm::sign(ptr->data->v, ptr->sym->data->v, point) > 0) // go to next 
			{
				current = ptr->prevOn();
				ptr = current;
				moved = false;
				continue;
			}
			ptr = ptr->sym->prevOn();
			moved = true;
		} while (!moved || ptr != current);
		return current;
	}
}
