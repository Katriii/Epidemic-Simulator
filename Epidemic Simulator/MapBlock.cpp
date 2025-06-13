#include "MapBlock.h"

MapBlock::MapBlock(std::vector<Vector2i> squaresFormingBlock, Size size, AreaType type)
{
	occupiedSquares = squaresFormingBlock;
	areaType = type;
	blockSize = size;
}

std::vector<Vector2i> MapBlock::GetOccupiedSquares() const
{
	return occupiedSquares;
}

Size MapBlock::GetBlockSize() const
{
	return blockSize;
}

AreaType MapBlock::GetAreaType() const
{
	return areaType;
}