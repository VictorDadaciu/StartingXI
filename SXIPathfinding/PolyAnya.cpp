#include "PolyAnya.h"

#include "CDT.h"
#include "Line.h"
#include <queue>
#include <functional>
#include <algorithm>

namespace sxi
{
	PolyAnya::PNode::PNode(const glm::vec2& root, float g, const glm::vec2& a, bool includeA, const glm::vec2& b, bool includeB, QuarterEdge* edge, glm::vec2 goal) : root(root), a(a), b(b), edge(edge) 
	{
		inclusion = (includeA * 0xFFFF0000) | (includeB * 0x0000FFFF);
		Line interval(a, b);
		if (interval.below(goal))
			goal = interval.mirror(goal);
		Ray rayA(root, a - root);
		Ray rayB(root, b - root);
		Ray rootGoal(root, goal - root);
		float h = ((rootGoal.between(rayA, rayB)) ? glm::length(rootGoal.dir) : fminf(glm::length(rayA.dir) + glm::length(goal - a), glm::length(rayB.dir) + glm::length(goal - b)));
		f = g + h;
	}

	std::vector<glm::vec2> PolyAnya::reconstructPath() const
	{
		std::vector<glm::vec2> path;
		path.push_back(goal);
		auto it = cameFrom.find(goal);
		while (it != cameFrom.end())
		{
			path.push_back(it->second);
			it = cameFrom.find(it->second);
		}
		std::reverse(path.begin(), path.end());
		return path;
	}

	PolyAnya::PolyAnya(const glm::vec2& start, const glm::vec2& goal, QuarterEdge* startEdge, QuarterEdge* goalEdge) : start(start), goal(goal)
	{
		gScores[start] = 0;
		constructGoalShape(goalEdge);
		constructInitialNodes(startEdge);
	}

	void PolyAnya::constructGoalShape(QuarterEdge* goalEdge)
	{
		QuarterEdge* ptr = goalEdge;
		do
		{
			if (!ptr->constrained)
				goalShape.insert(ptr);
			ptr = ptr->sym->prevOn();
		} while (ptr != goalEdge);
	}

	void PolyAnya::constructInitialNodes(QuarterEdge* startEdge)
	{
		float startGScore = gScores[start];
		QuarterEdge* ptr = startEdge;
		do
		{
			if (!ptr->constrained)
				open.emplace(start, startGScore, ptr->data->v, true, ptr->sym->data->v, true, ptr->sym, goal);
			ptr = ptr->sym->prevOn();
		} while (ptr != startEdge);
	}

	inline PolyAnya::PNode PolyAnya::pop()
	{
		PNode node = open.top();
		open.pop();
		return node;
	}

	bool PolyAnya::updateGScore(const glm::vec2& root, const glm::vec2& next)
	{
		if (next == root || cameFrom[next] == root)
			return true;
		auto g = gScores.find(next);
		float newG = gScores[root] + glm::length(next - root);
		if (g == gScores.end() || g->second > newG)
		{
			gScores[next] = newG;
			cameFrom[next] = root;
			return true;
		}
		return false;
	}

	void PolyAnya::emplace(const glm::vec2& root, const glm::vec2& a, bool includeA, const glm::vec2& b, bool includeB, QuarterEdge* edge, bool enteringGoalShape)
	{
		if (enteringGoalShape)
		{
			Ray goalRay(root, goal - root);
			Ray startRay(root, a - root);
			Ray endRay(root, b - root);
			if (goalRay.between(startRay, endRay) && updateGScore(root, goal))
			{
				open.emplace(goal, gScores[goal], goal, false, goal, false, nullptr, goal);
			}
		}
		else
		{
			open.emplace(root, gScores[root], a, includeA, b, includeB, edge, goal);
		}
	}

	std::vector<glm::vec2> PolyAnya::run()
	{
		while (!open.empty())
		{
			PNode node = pop();
			if (node.root == goal) // popped goal node, found path
				return reconstructPath();

#ifdef DEBUG
			metadata.intervalsExpanded.push_back(node.a);
			metadata.intervalsExpanded.push_back(node.b);
#endif // DEBUG

			QuarterEdge* ptr = node.edge->sym->prevOn();
			bool includeA = aRoot(node);
			bool includeB = bRoot(node);
			bool enteringGoalShape = goalShape.find(node.edge) != goalShape.end();
			Ray rayA(node.root, node.a - node.root);
			Ray rayB(node.root, node.b - node.root);
			// traverse new shape
			while (ptr != node.edge)
			{
				if (!ptr->constrained || enteringGoalShape) // impassable edges are uninteresting for expansion, unless goal shape
				{
					glm::vec2 dataRoot = ptr->data->v - node.root; // root -> beginning of edge
					glm::vec2 symRoot = ptr->sym->data->v - node.root; // root -> end of edge
					float dataCrossA = vec::cross(dataRoot, rayA.dir);
					float dataCrossB = vec::cross(dataRoot, rayB.dir);
					float symCrossA = vec::cross(symRoot, rayA.dir);
					float symCrossB = vec::cross(symRoot, rayB.dir);
					bool dataLeftOfCone = dataCrossA < 0;
					bool dataOnLeft = dataCrossA == 0;
					bool symLeftOfCone = symCrossA < 0;
					bool symOnLeft = symCrossA == 0;
					bool dataRightOfCone = dataCrossB > 0;
					bool dataOnRight = dataCrossB == 0;
					bool symRightOfCone = symCrossB > 0;
					bool symOnRight = symCrossB == 0;
					bool dataBetween = !dataLeftOfCone && !dataOnLeft && !dataRightOfCone && !dataOnRight;
					bool symBetween = !symLeftOfCone && !symOnLeft && !symRightOfCone && !symOnRight;
					glm::vec2 intA, intB;
					Line edge(ptr->data->v, ptr->sym->data->v);
					if (dataLeftOfCone && !symLeftOfCone && !symOnLeft)
					{
						rayA.intersects(edge, intA);
					}
					if (!dataRightOfCone && !dataOnRight && symRightOfCone)
					{
						rayB.intersects(edge, intB);
					}
					if ((dataLeftOfCone || dataOnLeft) && (symLeftOfCone || symOnLeft))
					{
						if (includeA)
							emplace(node.a, ptr->data->v, true, ptr->sym->data->v, true, ptr->sym, enteringGoalShape);
					}
					else if (dataLeftOfCone && (symOnRight || symBetween))
					{
						if (includeA)
							emplace(node.a, ptr->data->v, true, intA, false, ptr->sym, enteringGoalShape);
						emplace(node.root, intA, false, ptr->sym->data->v, true, ptr->sym, enteringGoalShape);
					}
					else if (dataLeftOfCone && symRightOfCone)
					{
						if (includeA)
							emplace(node.a, ptr->data->v, true, intA, false, ptr->sym, enteringGoalShape);
						emplace(node.root, intA, false, intB, false, ptr->sym, enteringGoalShape);
						if (includeB)
							emplace(node.b, intB, false, ptr->sym->data->v, true, ptr->sym, enteringGoalShape);
					}
					else if ((dataOnLeft || dataBetween) && (symOnRight || symBetween))
					{
						emplace(node.root, ptr->data->v, true, ptr->sym->data->v, true, ptr->sym, enteringGoalShape);
					}
					else if ((dataBetween || dataOnLeft) && symRightOfCone)
					{
						emplace(node.root, ptr->data->v, true, intB, false, ptr->sym, enteringGoalShape);
						if (includeB)
							emplace(node.b, intB, false, ptr->sym->data->v, true, ptr->sym, enteringGoalShape);
					}
					else if ((dataRightOfCone || dataOnRight) && (symRightOfCone || symOnRight))
					{
						if (includeB)
							emplace(node.b, ptr->data->v, true, ptr->sym->data->v, true, ptr->sym, enteringGoalShape);
					}
				}
				ptr = ptr->sym->prevOn();
			}
		}
		// no path found
		return std::vector<glm::vec2>();
	}

	std::vector<glm::vec2> runPolyAnya(const glm::vec2& start, const glm::vec2& goal, QuarterEdge* startEdge, QuarterEdge* goalEdge, PAMetadata& metadata)
	{
		// check if start and goal are in same shape
		QuarterEdge* ptr = startEdge;
		do
		{
			if (ptr == goalEdge)
				return std::vector<glm::vec2> {
					start,
					goal
				};
			ptr = ptr->sym->prevOn();
		} while (ptr != startEdge);
		PolyAnya polyAnya(start, goal, startEdge, goalEdge);
		std::chrono::steady_clock::time_point before = std::chrono::steady_clock::now();
		std::vector<glm::vec2> retVal = polyAnya.run();
		polyAnya.metadata.timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - before);
		metadata = polyAnya.metadata;
		return retVal;
	}
}