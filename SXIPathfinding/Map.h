#pragma once

#include "MapShape.h"
#include <memory>

namespace sxi
{
	enum ShapeType
	{
		Default = 0,
		Count = 1
	};

	class CDT;
	class RST;
	class Map
	{
	public:
		static Map& instance()
		{
			static Map map;
			return map;
		}

		bool initialize(float, float, float, float);

		~Map() = default;

		Map(const Map&) = delete;
		void operator=(const Map&) = delete;

		void insert(ShapeType, const std::vector<glm::vec2>&);
		std::vector<glm::vec2> remove(ShapeType, uint32_t);
		std::vector<glm::vec2> remove(const glm::vec2&);
		std::vector<glm::vec2> findPath(float, float, float, float) const;
		std::vector<glm::vec2> findPath(const glm::vec2&, const glm::vec2&) const;
		bool inside(const glm::vec2&) const;
		bool inside(float, float) const;

		std::vector<std::pair<AABB, int8_t>> collectRST() const;
		void collectCDT(std::vector<glm::vec2>&, std::vector<bool>&, std::vector<int>&, bool) const;

	private:
		Map();

		std::vector<std::vector<MapShape>> shapes;
		std::unique_ptr<CDT> cdt;
		std::unique_ptr<RST> rst;
		bool initialized = false;

		friend class RST;
	};
}