#pragma once

#include "AABB.h"
#include <vector>
#include <queue>

namespace sxi
{
	struct RSTLeaf
	{
		uint32_t type, index;
		RSTLeaf(uint32_t type, uint32_t index) : type(type), index(index) {}
	};

	struct QuarterEdge;
	class RSTBranch;
	class RSTNode
	{
	public:
		RSTNode() = default;
		RSTNode(void*, const AABB&);
		RSTNode(const RSTNode&);
		RSTNode(const RSTNode&&);

		inline void operator=(const RSTNode& other) 
		{  
			this->aabb = other.aabb;
			this->data = other.data;
		}

		inline bool operator==(const RSTNode& other)
		{
			return data == other.data;
		}

		inline bool operator==(RSTNode&& other)
		{
			return data == other.data;
		}

		void recreateBoundingBox();
		RSTNode split(RSTNode&&);
		std::vector<RSTNode> overflow(RSTNode&&, int8_t level);
		int chooseSubtree(const AABB&, int8_t);

	private:
		std::vector<std::pair<AABB, int8_t>> collect(int8_t) const;

		AABB aabb{};
		void* data{};

		friend class RST;
		friend class RSTBranch;
		friend struct RSTNodeLevelValue;
	};

	struct RSTNodeLevelValue
	{
		RSTNode node;
		float f{};
		int8_t atLevel;

		RSTNodeLevelValue() = default;
		RSTNodeLevelValue(const RSTNode& node, int8_t atLevel) : node(node), atLevel(atLevel) {}
		RSTNodeLevelValue(const RSTNode& node, int8_t atLevel, float f) : node(node), f(f), atLevel(atLevel) {}

		inline bool operator>(const RSTNodeLevelValue& other) const { return f > other.f; }
		inline bool operator<(const RSTNodeLevelValue& other) const { return f < other.f; }
	};

	class RST
	{
	public:
		static const uint8_t M = 5;
		static const uint8_t m = 2;
		static const uint8_t p = 2;
		static uint32_t overflow;
		static int8_t depth;

		RST();
		~RST() = default;

		QuarterEdge* getBestEdge(const glm::vec2&) const;
		QuarterEdge* getBestEdge(const glm::vec2&, glm::vec2&) const;
		void insert(RSTLeaf*, AABB&&);
		void remove(RSTLeaf*, AABB&&);
		bool inside(const glm::vec2&) const;
		RSTLeaf* shapeAt(const glm::vec2&) const;

		std::vector<std::pair<AABB, int8_t>> collect() const;

	private:
		void insertAt(RSTNode*, RSTNode&&, int8_t, const int8_t);
		bool findLeafAndRemove(RSTNode*, RSTLeaf*, AABB&&, int8_t);
		bool condenseTree(RSTNode*, int, int8_t);
		void deepen(RSTNode&& newNode);

		std::queue<RSTNodeLevelValue> insertionQueue{};
		RSTNode* root = nullptr;
	};

	class RSTBranch
	{
	public:
		RSTBranch();
		~RSTBranch();

		void add(const RSTNode& node)
		{
			if (n >= RST::M)
				throw std::exception("Map node child array size exceeded");
			children[n++] = node;
		}
		inline bool tooEmpty() const { return n < RST::m; }
		inline uint8_t size() const { return n; }
		inline bool full() const { return n == RST::M; }
		inline void clear() { n = 0; }
		inline RSTNode* at(uint8_t index) const 
		{ 
			if (index < 0 || index >= n)
				throw std::exception("Cannot access R* tree's branch's child at that index");
			return &children[index];
		}
		inline void remove(uint8_t index)
		{
			if (index < 0 || index >= n)
				throw std::exception("Cannot access R* tree's branch's child at that index");
			children[index] = children[--n];
		}

	private:
		RSTNode* children;
		uint8_t n{};

		friend class RST;
		friend class RSTNode;
	};
}

