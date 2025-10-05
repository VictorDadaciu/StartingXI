#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <chrono>

#include "SXIMath/Vec.h"

namespace sxi
{
	struct PAMetadata
	{
		std::vector<glm::vec2> intervalsExpanded;
		std::chrono::microseconds timeTaken;
	};

	struct QuarterEdge;

	class PolyAnya
	{
		class PNode
		{
		public:
			PNode(const glm::vec2&, float, const glm::vec2&, bool, const glm::vec2&, bool, QuarterEdge*, glm::vec2);

			glm::vec2 root = SXI_VEC2_MAX;
			glm::vec2 a = SXI_VEC2_MAX;
			glm::vec2 b = SXI_VEC2_MAX;
			QuarterEdge* edge = nullptr;
			uint32_t inclusion = 0;
			float f;

			inline bool operator<(const PNode& other) const
			{
				return f < other.f;
			}

			inline bool operator>(const PNode& other) const
			{
				return f > other.f;
			}

			inline bool includeA() const
			{
				return inclusion & 0xFFFF0000;
			}

			inline bool includeB() const
			{
				return inclusion & 0x0000FFFF;
			}
		};
	public:
		PolyAnya(const glm::vec2&, const glm::vec2&, QuarterEdge*, QuarterEdge*);

		std::vector<glm::vec2> run();

		PAMetadata metadata;
	private:
		void constructGoalShape(QuarterEdge*);
		void constructInitialNodes(QuarterEdge*);
		inline PNode pop();
		std::vector<glm::vec2> reconstructPath() const;
		bool updateGScore(const glm::vec2&, const glm::vec2&);
		inline bool aRoot(const PNode& node)
		{
			if (!node.includeA())
				return false;
			return updateGScore(node.root, node.a);
		}
		inline bool bRoot(const PNode& node)
		{
			if (!node.includeB())
				return false;
			return updateGScore(node.root, node.b);
		}
		void emplace(const glm::vec2&, const glm::vec2&, bool, const glm::vec2&, bool, QuarterEdge*, bool);

		std::priority_queue<PNode, std::vector<PNode>, std::greater<PNode>> open;
		std::unordered_map<glm::vec2, glm::vec2> cameFrom;
		std::unordered_map<glm::vec2, float> gScores;
		std::unordered_set<QuarterEdge*> goalShape;
		glm::vec2 start, goal;
	};

	std::vector<glm::vec2> runPolyAnya(const glm::vec2&, const glm::vec2&, QuarterEdge*, QuarterEdge*, PAMetadata&);
}

