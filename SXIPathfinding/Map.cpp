#include "Map.h"

#include "RStarTree.h"
#include "CDT.h"
#include <algorithm>
#include "PolyAnya.h"

namespace sxi
{
	Map::Map() : shapes(ShapeType::Count), cdt(std::make_unique<CDT>()), rst(std::make_unique<RST>()), initialized(false) {}

	bool Map::initialize(float x1, float y1, float x2, float y2)
	{
		cdt.reset(new CDT(x1, y1, x2, y2));
		rst.reset(new RST());

		initialized = true;
		return initialized;
	}

	void Map::insert(ShapeType type, const std::vector<glm::vec2>& points)
	{
		if (!initialized)
			throw std::exception("Must initialize map before inserting points");

		if (type < 0 || type >= ShapeType::Count)
			throw std::exception("Invalid shape type");

		std::vector<MapShape>& shapesOfType = shapes[type];
		uint32_t index = shapesOfType.size();
		MapShape newShape = cdt->insertShape(std::move(points), rst->getBestEdge(points[0]));
		newShape.leaf = new RSTLeaf(type, index);
		rst->insert(newShape.leaf, newShape.generateBoundingBox());
		shapesOfType.push_back(newShape);
	}

	std::vector<glm::vec2> Map::remove(const glm::vec2& point)
	{
		if (!initialized)
			throw std::exception("Must initialize map before removing points");

		RSTLeaf* leaf = rst->shapeAt(point);
		if (!leaf)
			return std::vector<glm::vec2>();

		return remove((ShapeType)leaf->type, leaf->index);
	}

	// TODO can do a lot here at removal, very inefficient
	std::vector<glm::vec2> Map::remove(ShapeType type, uint32_t index)
	{
		if (!initialized)
			throw std::exception("Must initialize map before removing points");

		if (type < 0 || type >= ShapeType::Count)
			throw std::exception("Invalid shape type");

		std::vector<MapShape>& shapesOfType = shapes[type];
		if (shapesOfType.empty())
			return std::vector<glm::vec2>();

		if (index < 0 || index >= shapesOfType.size())
			throw std::exception("Invalid shape index");

		MapShape shapeToRemove = shapesOfType[index];
		rst->remove(shapeToRemove.leaf, shapeToRemove.generateBoundingBox());
		if (index != shapesOfType.size() - 1)
		{
			shapesOfType[index] = shapesOfType[shapesOfType.size() - 1];
			shapesOfType[index].leaf->index = index;
		}
		shapesOfType.pop_back();
		delete shapeToRemove.leaf;
		return cdt->removeShape(shapeToRemove);
	}

	std::vector<glm::vec2> Map::findPath(float x, float y, float gx, float gy) const
	{
		PAMetadata metadata;
		glm::vec2 start, goal;
		sxi::QuarterEdge* startBestEdge = rst->getBestEdge(glm::vec2(x, y), start);
		sxi::QuarterEdge* goalBestEdge = rst->getBestEdge(glm::vec2(gx, gy), goal);
		sxi::QuarterEdge* startPolyEdge = cdt->find(start, startBestEdge);
		sxi::QuarterEdge* goalPolyEdge = cdt->find(goal, goalBestEdge);
		return sxi::runPolyAnya(start, goal, startPolyEdge, goalPolyEdge, metadata);
	}

	std::vector<glm::vec2> Map::findPath(const glm::vec2& tryStart, const glm::vec2& tryGoal) const
	{
		PAMetadata metadata;
		glm::vec2 start, goal;
		sxi::QuarterEdge* startBestEdge = rst->getBestEdge(tryStart, start);
		sxi::QuarterEdge* goalBestEdge = rst->getBestEdge(tryGoal, goal);
		sxi::QuarterEdge* startPolyEdge = cdt->find(start, startBestEdge);
		sxi::QuarterEdge* goalPolyEdge = cdt->find(goal, goalBestEdge);
		return sxi::runPolyAnya(start, goal, startPolyEdge, goalPolyEdge, metadata);
	}

	bool Map::inside(float x, float y) const
	{
		return rst->inside(glm::vec2(x, y));
	}

	bool Map::inside(const glm::vec2& point) const
	{
		return rst->inside(point);
	}

	std::vector<std::pair<AABB, int8_t>> Map::collectRST() const
	{
		return rst->collect();
	}

	void Map::collectCDT(std::vector<glm::vec2>& points, std::vector<bool>& constrained, std::vector<int>& edges, bool onlyOn) const
	{
		cdt->collect(points, constrained, edges, onlyOn);
	}
}