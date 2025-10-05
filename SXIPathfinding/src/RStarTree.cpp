#include "RStarTree.h"

#include "Map.h"
#include "MapShape.h"
#include "CDT.h"

#include <cstring>
#include <limits.h>
#include <algorithm>
#include <queue>

namespace sxi
{
	int8_t RST::depth = -1;
	uint32_t RST::overflow = 0x00000001;

	RST::RST() : root(nullptr) {}

	RSTNode::RSTNode(void* data, const AABB& aabb) : aabb(aabb), data(data) {}

	RSTNode::RSTNode(const RSTNode& node) : aabb(node.aabb), data(node.data) {}

	RSTNode::RSTNode(const RSTNode&& node) : aabb(node.aabb), data(node.data) {}

	static inline bool hasOverflowed(int8_t level)
	{
		return RST::overflow & (1 << level);
	}

	static inline void overflowed(int8_t level)
	{
		RST::overflow = RST::overflow | (1 << level);
	}

	RSTBranch::RSTBranch()
	{
		children = new RSTNode[RST::M];
	}

	RSTBranch::~RSTBranch()
	{
		delete[] children;
	}

	QuarterEdge* RST::getBestEdge(const glm::vec2& point) const
	{
		glm::vec2 temp;
		return getBestEdge(point, temp);
	}

	RSTLeaf* RST::shapeAt(const glm::vec2& point) const
	{
		if (!root)
			return nullptr;

		std::queue<RSTNodeLevelValue> inside;
		if (root->aabb.inside(point))
		{
			if (RST::depth == -1)
			{
				RSTLeaf* leaf = static_cast<RSTLeaf*>(root->data);
				MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
				if (mapShape.inside(point))
					return leaf;
				else
					return nullptr;
			}
			inside.emplace(*root, 0);
		}
		else
		{
			return nullptr;
		}

		while (!inside.empty())
		{
			RSTNodeLevelValue element = inside.front();
			inside.pop();
			RSTBranch* branch = static_cast<RSTBranch*>(element.node.data);
			for (int i = 0; i < branch->n; i++)
			{
				if (branch->children[i].aabb.inside(point))
				{
					if (element.atLevel == RST::depth)
					{
						RSTLeaf* leaf = static_cast<RSTLeaf*>(branch->children[i].data);
						MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
						if (mapShape.inside(point))
							return leaf;
					}
					else
					{
						inside.emplace(branch->children[i], element.atLevel + 1);
					}
				}
			}
		}
		return nullptr;
	}

	bool RST::inside(const glm::vec2& point) const
	{
		if (!root)
			return false;

		std::queue<RSTNodeLevelValue> inside;
		if (root->aabb.inside(point))
		{
			if (RST::depth == -1)
			{
				RSTLeaf* leaf = static_cast<RSTLeaf*>(root->data);
				MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
				if (mapShape.inside(point))
					return true;
				else
					return false;
			}
			inside.emplace(*root, 0);
		}
		else
		{
			return false;
		}

		while (!inside.empty())
		{
			RSTNodeLevelValue element = inside.front();
			inside.pop();
			RSTBranch* branch = static_cast<RSTBranch*>(element.node.data);
			for (int i = 0; i < branch->n; i++)
			{
				if (branch->children[i].aabb.inside(point))
				{
					if (element.atLevel == RST::depth)
					{
						RSTLeaf* leaf = static_cast<RSTLeaf*>(branch->children[i].data);
						MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
						if (mapShape.inside(point))
							return true;
					}
					else
					{
						inside.emplace(branch->children[i], element.atLevel + 1);
					}
				}
			}
		}
		return false;
	}

	QuarterEdge* RST::getBestEdge(const glm::vec2& point, glm::vec2& newPoint) const
	{
		newPoint = point;
		if (!root)
			return nullptr;

		if (depth == -1)
		{
			RSTLeaf* leaf = static_cast<RSTLeaf*>(root->data);
			MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
			if (root->aabb.inside(point) && mapShape.inside(point))
				return mapShape.closestEdgeForPointInside(point, newPoint);
			else
				return mapShape.closestEdgeForPointOutside(point);
		}

		std::priority_queue<RSTNodeLevelValue, std::vector<RSTNodeLevelValue>, std::greater<RSTNodeLevelValue>> open;
		std::queue<RSTNodeLevelValue> inside;
		if (root->aabb.inside(point))
			inside.emplace(*root, 0);
		else
			open.emplace(*root, 0);

		while (!inside.empty()) // populate open queue
		{
			RSTNodeLevelValue element = inside.front();
			inside.pop();
			RSTBranch* branch = static_cast<RSTBranch*>(element.node.data);
			for (int i = 0; i < branch->n; i++)
			{
				if (branch->children[i].aabb.inside(point))
				{
					if (element.atLevel == RST::depth)
					{
						RSTLeaf* leaf = static_cast<RSTLeaf*>(branch->children[i].data);
						MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
						if (mapShape.inside(point))
							return mapShape.closestEdgeForPointInside(point, newPoint);
						else
							open.emplace(branch->children[i], element.atLevel + 1, branch->children[i].aabb.sqrDistance(point));
					}
					else
					{
						inside.emplace(branch->children[i], element.atLevel + 1);
					}
				}
				else
				{
					open.emplace(branch->children[i], element.atLevel + 1, branch->children[i].aabb.sqrDistance(point));
				}
			}
		}
		while (!open.empty())
		{
			RSTNodeLevelValue element = open.top();
			open.pop();
			if (element.atLevel == RST::depth + 1)
			{
				RSTLeaf* leaf = static_cast<RSTLeaf*>(element.node.data);
				MapShape& mapShape = Map::instance().shapes[leaf->type][leaf->index];
				return mapShape.closestEdgeForPointOutside(point);
			}
			RSTBranch* branch = static_cast<RSTBranch*>(element.node.data);
			for (int i = 0; i < branch->n; i++)
				open.emplace(branch->children[i], element.atLevel + 1, branch->children[i].aabb.sqrDistance(point));
		}
		return nullptr;
	}

	void RST::insert(RSTLeaf* leaf, AABB&& aabb)
	{
		overflow = 0x00000001;
		RSTNode node(leaf, std::move(aabb));
		if (!root)
		{
			root = new RSTNode();
			*root = node;
			return;
		}

		if (depth == -1)
		{
			deepen(std::move(node));
			return;
		}

		insertAt(root, std::move(node), 0, RST::depth);
		while (!insertionQueue.empty())
		{
			RSTNodeLevelValue pair = insertionQueue.front();
			insertionQueue.pop();
			insertAt(root, std::move(pair.node), 0, pair.atLevel);
		}
	}

	void RST::insertAt(RSTNode* node, RSTNode&& newNode, int8_t level, const int8_t desiredLevel)
	{
		RSTBranch* branch = static_cast<RSTBranch*>(node->data);
		if (level == desiredLevel)
		{
			if (!branch->full())
			{
				branch->add(std::move(newNode));
				node->aabb = AABB::combine(node->aabb, newNode.aabb);
			}
			else if (hasOverflowed(level))
			{
				RSTNode splitNode = node->split(std::move(newNode));
				if (level == 0)
				{
					deepen(std::move(splitNode));
					return;
				}
				insertionQueue.emplace(splitNode, level - 1);
			}
			else
			{
				overflowed(level);
				std::vector<RSTNode> overflowVec = node->overflow(std::move(newNode), level);
				for (RSTNode overflowNode : overflowVec)
					insertionQueue.emplace(overflowNode, level);
				node->recreateBoundingBox();
			}
		}
		else
		{
			int index = node->chooseSubtree(newNode.aabb, level);
			insertAt(branch->at(index), std::move(newNode), ++level, desiredLevel);
			node->recreateBoundingBox();
		}
	}

	void RST::deepen(RSTNode&& newNode)
	{
		RSTBranch* branch = new RSTBranch();
		AABB combined = AABB::combine(root->aabb, newNode.aabb);
		branch->add(*root);
		branch->add(newNode);
		*root = RSTNode(branch, combined);
		++depth;
	}

	void RSTNode::recreateBoundingBox()
	{
		RSTBranch* branch = static_cast<RSTBranch*>(data);
		aabb = AABB(branch->children[0].aabb);
		for (int i = 1; i < branch->n; i++)
			aabb = AABB::combine(aabb, branch->children[i].aabb);
	}

	static std::vector<RSTNode> mergeWithNewNode(RSTNode* children, RSTNode&& newNode)
	{
		std::vector<RSTNode> allNodes(RST::M + 1);
		memcpy(allNodes.data(), children, RST::M * sizeof(RSTNode));
		allNodes[RST::M] = newNode;
		return allNodes;
	}

	std::vector<RSTNode> RSTNode::overflow(RSTNode&& newNode, int8_t level)
	{
		RSTBranch* branch = static_cast<RSTBranch*>(data);
		glm::vec2 center = aabb.center();
		aabb = AABB::invalid();
		std::vector<RSTNode> allChildren = mergeWithNewNode(branch->children, std::move(newNode));
		branch->clear();
		std::vector<RSTNodeLevelValue> nodesWithDists(allChildren.size());
		std::transform(allChildren.cbegin(), allChildren.cend(), nodesWithDists.begin(), [level, center](const RSTNode& node) {return RSTNodeLevelValue(node, level, glm::sqrLength(node.aabb.center() - center)); });
		// sort by distance to center of current parent aabb
		std::sort(nodesWithDists.begin(), nodesWithDists.end());
		// re-add closest children to current branch
		int remainderIndex = RST::M + 1 - RST::p;
		for (int i = 0; i < RST::M + 1 - RST::p; i++)
		{
			aabb = AABB::combine(aabb, nodesWithDists[i].node.aabb);
			branch->add(std::move(nodesWithDists[i].node));
		}
		// construct overflow vector
		std::vector<RSTNode> retVal(RST::p);
		for (int i = 0; i < RST::p; i++)
			retVal[i] = nodesWithDists[i + remainderIndex].node;
		// return last p children
		return retVal;
	}

	RSTNode RSTNode::split(RSTNode&& newNode)
	{
		RSTBranch* branch = static_cast<RSTBranch*>(data);
		aabb = AABB::invalid();
		std::vector<RSTNode> sortedChildren = mergeWithNewNode(branch->children, std::move(newNode));
		branch->clear();
		RSTBranch* splitBranch = new RSTBranch();
		RSTNode splitNode = RSTNode(splitBranch, AABB::invalid());
		// get split axis
		{
			std::vector<RSTNode> sortedByX = sortedChildren;
			std::sort(sortedByX.begin(), sortedByX.end(),
					  [](const RSTNode& l, const RSTNode& r) 
					  {
						  float llx = l.aabb.topLeft.x;
						  float rlx = r.aabb.topLeft.x;
						  float lrx = l.aabb.botRight.x;
						  float rrx = r.aabb.botRight.x;
						  return llx < rlx || (llx == rlx && lrx < rrx);
					  });
			std::vector<RSTNode> sortedByY = sortedChildren;
			std::sort(sortedByY.begin(), sortedByY.end(),
					  [](const RSTNode& l, const RSTNode& r) 
					  {
						  float lty = l.aabb.topLeft.y;
						  float rty = r.aabb.topLeft.y;
						  float lby = l.aabb.botRight.y;
						  float rby = r.aabb.botRight.y;
						  return lty < rty || (lty == rty && lby < rby);
					  });
			float goodnessX = std::numeric_limits<float>::max(), goodnessY = std::numeric_limits<float>::max();
			// x axis
			{
				AABB frontBB = AABB::invalid();
				AABB backBB = AABB::invalid();
				uint8_t until = RST::M - RST::m + 1;
				for (int i = 0; i < RST::m; i++)
				{
					frontBB = AABB::combine(frontBB, sortedByX[i].aabb);
				}
				for (int i = until; i < sortedByX.size(); i++)
				{
					backBB = AABB::combine(backBB, sortedByX[i].aabb);
				}
				for (int i = RST::m; i < until + 1; i++)
				{
					AABB accFrontBB = frontBB;
					AABB accBackBB = backBB;
					for (int j = RST::m; j < i; j++)
					{
						accFrontBB = AABB::combine(accFrontBB, sortedByX[j].aabb);
					}
					for (int j = i; j < until; j++)
					{
						accBackBB = AABB::combine(accBackBB, sortedByX[j].aabb);
					}
					float margin = accFrontBB.margin() + accBackBB.margin();
					if (margin < goodnessX)
					{
						goodnessX = margin;
					}
				}
			}
			// y axis
			{
				AABB frontBB = AABB::invalid();
				AABB backBB = AABB::invalid();
				uint8_t until = RST::M - RST::m + 1;
				for (int i = 0; i < RST::m; i++)
				{
					frontBB = AABB::combine(frontBB, sortedByY[i].aabb);
				}
				for (int i = until; i < sortedByY.size(); i++)
				{
					backBB = AABB::combine(backBB, sortedByY[i].aabb);
				}
				for (int i = RST::m; i < until + 1; i++)
				{
					AABB accFrontBB = frontBB;
					AABB accBackBB = backBB;
					for (int j = RST::m; j < i; j++)
					{
						accFrontBB = AABB::combine(accFrontBB, sortedByY[j].aabb);
					}
					for (int j = i; j < until; j++)
					{
						accBackBB = AABB::combine(accBackBB, sortedByY[j].aabb);
					}
					float margin = accFrontBB.margin() + accBackBB.margin();
					if (margin < goodnessY)
					{
						goodnessY = margin;
					}
				}
			}
			sortedChildren = goodnessX < goodnessY ? std::move(sortedByX) : std::move(sortedByY);
		}
		// choose distribution
		{
			float minOverlap = std::numeric_limits<float>::max(), minArea = std::numeric_limits<float>::max();
			uint8_t bestIndex = RST::m;
			uint8_t until = RST::M - RST::m + 1;
			for (int i = 0; i < RST::m; i++)
			{
				aabb = AABB::combine(aabb, sortedChildren[i].aabb);
				branch->add(std::move(sortedChildren[i]));
			}
			for (int i = until; i < sortedChildren.size(); i++)
			{
				splitNode.aabb = AABB::combine(splitNode.aabb, sortedChildren[i].aabb);
				splitBranch->add(std::move(sortedChildren[i]));
			}
			for (int i = RST::m; i < until + 1; i++)
			{
				AABB accFrontBB = aabb;
				AABB accBackBB = splitNode.aabb;
				for (int j = RST::m; j < i; j++)
				{
					accFrontBB = AABB::combine(accFrontBB, sortedChildren[j].aabb);
				}
				for (int j = i; j < until; j++)
				{
					accBackBB = AABB::combine(accBackBB, sortedChildren[j].aabb);
				}
				float overlap = accFrontBB.overlapArea(accBackBB);
				float area = accFrontBB.area() + accBackBB.area();
				if (overlap < minOverlap || (overlap == minOverlap && area < minArea))
				{
					bestIndex = i;
					minOverlap = overlap;
					minArea = area;
				}
			}
			for (int i = RST::m; i < bestIndex; i++)
			{
				aabb = AABB::combine(aabb, sortedChildren[i].aabb);
				branch->add(std::move(sortedChildren[i]));
			}
			for (int i = bestIndex; i < until; i++)
			{
				splitNode.aabb = AABB::combine(splitNode.aabb, sortedChildren[i].aabb);
				splitBranch->add(std::move(sortedChildren[i]));
			}
		}
		return splitNode;
	}

	void RST::remove(RSTLeaf* leaf, AABB&& aabb)
	{
		overflow = 0x00000001;
		if (!root)
			return;

		if (depth == -1)
		{
			if (root->data == leaf)
			{
				delete root;
				root = nullptr;
			}
			return;
		}

		if (root->aabb.intersects(aabb))
		{
			findLeafAndRemove(root, leaf, std::move(aabb), 0);
			while (!insertionQueue.empty())
			{
				RSTNodeLevelValue nodeLevel = insertionQueue.front();
				insertionQueue.pop();
				insertAt(root, std::move(nodeLevel.node), 0, nodeLevel.atLevel);
			}
			while (depth > -1)
			{
				RSTBranch* rootBranch = static_cast<RSTBranch*>(root->data);
				if (rootBranch->size() == 1)
				{
					*root = rootBranch->children[0];
					delete rootBranch;
					--depth;
				}
				else break;
			}
		}
	}

	bool RST::condenseTree(RSTNode* node, int index, int8_t level)
	{
		RSTBranch* branch = static_cast<RSTBranch*>(node->data);
		branch->remove(index);
		if (!branch->tooEmpty() || level == 0)
		{
			node->recreateBoundingBox();
			return false;
		}
		else
		{
			for (int i = 0; i < branch->n; i++)
				insertionQueue.emplace(branch->children[i], level);
			delete branch;
			return true;
		}
	}

	bool RST::findLeafAndRemove(RSTNode* node, RSTLeaf* leaf, AABB&& aabb, int8_t level)
	{
		RSTBranch* branch = static_cast<RSTBranch*>(node->data);
		if (level == RST::depth)
		{
			for (int i = 0; i < branch->n; i++)
				if (branch->children[i].data == leaf)
					return condenseTree(node, i, level);
			return false;
		}

		for (int i = 0; i < branch->n; i++)
		{
			RSTNode* child = branch->at(i);
			if (child->aabb.intersects(aabb) && findLeafAndRemove(child, leaf, std::move(aabb), level + 1))
				return condenseTree(node, i, level);
		}
		return false;
	}

	int RSTNode::chooseSubtree(const AABB& aabb, int8_t level)
	{
		RSTBranch* branch = static_cast<RSTBranch*>(data);
		uint8_t index = RST::M + 1;
		float minArea = std::numeric_limits<float>::max();
		float minEnlargement = std::numeric_limits<float>::max();
		float minOverlapIncrease = std::numeric_limits<float>::max();
		for (int i = 0; i < branch->n; i++)
		{
			AABB childBB = branch->children[i].aabb;
			float currentArea = childBB.area();
			AABB newBB = AABB::combine(aabb, childBB);
			float tempArea = newBB.area();
			float tempEnlargement = tempArea - currentArea;
			float tempOverlapIncrease = 0;
			if (level == RST::depth - 1)
			{
				float currentOverlap = 0;
				float newOverlap = 0;
				for (int j = 0; j < branch->n; j++)
				{
					if (i != j)
					{
						AABB childBB2 = branch->children[j].aabb;
						currentOverlap += childBB.overlapArea(childBB2);
						newOverlap += newBB.overlapArea(childBB2);
					}
				}
				tempOverlapIncrease = newOverlap - currentOverlap;
			}
			if (tempOverlapIncrease < minOverlapIncrease || 
				((tempOverlapIncrease == minOverlapIncrease && tempEnlargement < minEnlargement) ||
				 (tempEnlargement == minEnlargement && tempArea < minArea)))
			{
				index = i;
				minOverlapIncrease = tempOverlapIncrease;
				minEnlargement = tempEnlargement;
				minArea = tempArea;
			}
		}
		return index;
	}

	std::vector<std::pair<AABB, int8_t>> RST::collect() const
	{
		if (!root)
			return std::vector<std::pair<AABB, int8_t>>();
		return root->collect(0);
	}

	std::vector<std::pair<AABB, int8_t>> RSTNode::collect(int8_t level) const
	{
		if (level == RST::depth + 1)
			return std::vector<std::pair<AABB, int8_t>>{ { aabb, level } };
		RSTBranch* branch = static_cast<RSTBranch*>(data);
		std::vector<std::pair<AABB, int8_t>> acc;
		for (int i = 0; i < branch->n; i++)
		{
			std::vector<std::pair<AABB, int8_t>> childAABBs = branch->children[i].collect(level + 1);
			acc.insert(acc.end(), childAABBs.begin(), childAABBs.end());
		}
		acc.push_back({ aabb, level });
		return acc;
	}
}