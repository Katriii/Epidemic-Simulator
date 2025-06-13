#pragma once
#include "raylib-cpp.hpp"
#include <iostream>
#include "Vector2i.h"

enum AreaType { GREEN_AREA, RESIDENTIAL_AREA, HOSPITAL, SHOPPING_AREA, WORKPLACE_AREA, AREA_COUNT };
enum Size { DOUBLE_VERTICAL, DOUBLE_HORIZONTAL, QUAD_SQUARE, STANDARD, SIZE_COUNT };

class MapBlock
{
private:
	std::vector<Vector2i> occupiedSquares;
	Size blockSize;
	AreaType areaType;

public:
	MapBlock(std::vector<Vector2i> squaresFormingBlock, Size size, AreaType type);
	std::vector<Vector2i> GetOccupiedSquares() const;
	Size GetBlockSize() const;
	AreaType GetAreaType() const;
};