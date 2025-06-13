#include "Map.h"
#include "Vector2i.h"
#include "Building.h"
#include "MapBlock.h"
#include <random>
#include <deque>
#include <algorithm>
#include <cmath>
#include <iostream>

// Return a random area type for the map block
AreaType GetRandomAreaType(std::mt19937& randomGenerator)
{
	// Weights in order for: RESIDENTIAL_AREA, GREEN_AREA, SHOPPING_AREA, WORKPLACE_AREA
	std::vector<int> weightDistribution = { 70, 18, 5, 7 };
	std::vector<AreaType> areaTypes = { AreaType::RESIDENTIAL_AREA, AreaType::GREEN_AREA, AreaType::SHOPPING_AREA, AreaType::WORKPLACE_AREA };

	std::discrete_distribution<> areaTypeDistribution(weightDistribution.begin(), weightDistribution.end());
	return areaTypes[areaTypeDistribution(randomGenerator)];
}

// Check if chosen block size fits in the map, select accompanying squares to it to form the block and remove them from unassigned squares
std::vector<Vector2i> FormFullBlock(Vector2i baseSquare, Size size, std::vector<Vector2i>* unassignedSquaresList)
{
	// Decide what squares are searched for depending on the size of the block
	std::vector<Vector2i> searched;

	switch (size)
	{
	case Size::DOUBLE_VERTICAL:
		searched = { { baseSquare.x, baseSquare.y + 1 } };
		break;

	case Size::DOUBLE_HORIZONTAL:
		searched = { { baseSquare.x + 1, baseSquare.y } };
		break;

	case Size::QUAD_SQUARE:
		searched = { {baseSquare.x + 1, baseSquare.y}, {baseSquare.x, baseSquare.y + 1}, {baseSquare.x + 1, baseSquare.y + 1} };
		break;

	default:
		// EXCEPTION MISSING
		break;
	}

	// Check whether neighbouring squares are valid to form the block
	std::vector<std::vector<Vector2i>::iterator> foundIters;
	for (const auto& searchedVector : searched)
	{
		auto it = std::find(unassignedSquaresList->begin(), unassignedSquaresList->end(), searchedVector);
		if (it == unassignedSquaresList->end())
			return {}; // At least one square is not valid, so the block cannot be formed
		foundIters.push_back(it);
	}

	// All neighbouring squares are valid, so we can delete them from unassigned squares
	std::vector<Vector2i> occupiedSquaresFound = { baseSquare };
	for (auto& it : foundIters)
	{
		occupiedSquaresFound.push_back(*it);
	}

	// If everything's valid, return all valid squares and erase them from unassigned squares
	std::sort(foundIters.rbegin(), foundIters.rend());
	for (auto it : foundIters)
	{
		unassignedSquaresList->erase(it);
	}

	return occupiedSquaresFound;

}

Map::Map(int populationSize, int residentsInBuildingLimit) : houseTexture("resources/house_icon.png"), shopTexture("resources/shop_icon.png"), workplaceTexture("resources/workplace_icon.png"), hospitalTexture("resources/hospital_icon.png")
{
	if (populationSize <= 0 || residentsInBuildingLimit <= 0)
		throw std::invalid_argument("Population size and residents in building limit must be greater than zero.");

	if (!houseTexture.IsValid() || !shopTexture.IsValid() || !workplaceTexture.IsValid() || !hospitalTexture.IsValid())
		throw std::runtime_error("Failed to load building textures.");

	int requiredResidentialBuildings = static_cast<int>(std::ceil(static_cast<float>(populationSize) / residentsInBuildingLimit));
	int totalSquares = static_cast<int>(std::ceil(requiredResidentialBuildings / 0.7f));

	mapSquareSize = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(totalSquares)))) + 1;
	mapPixelSize = mapSquareSize * SQUARE_WIDTH + (mapSquareSize + 1) * ROAD_WIDTH;
}

// Generate the map blocks and buildings
void Map::GenerateMap()
{
	// Prepare RNG
	std::random_device randomDevice;
	std::mt19937 randomGenerator(randomDevice());

	// Determine number of attempts to be taken when placing large blocks on the map
	int largeBlocksPlacementAttempts = static_cast<int>(std::round(mapSquareSize * mapSquareSize * LARGE_BLOCKS_PLACEMENT_INTENSITY));

	// Prepare list for map squares and shuffle it
	std::vector<Vector2i> unassignedSquaresList;
	for (int i = 0; i < mapSquareSize; i++)
		for (int j = 0; j < mapSquareSize; j++)
			unassignedSquaresList.push_back({ i, j });
	std::shuffle(unassignedSquaresList.begin(), unassignedSquaresList.end(), randomGenerator);

	// Generate hospital block at the center of the map
	Vector2i originHospitalSquare = { static_cast<int>(mapSquareSize / 2), static_cast<int>(mapSquareSize / 2) };
	MapBlock hospitalBlock({ originHospitalSquare }, Size::STANDARD, AreaType::HOSPITAL);
	mapBlocksList.push_back(hospitalBlock);
	// Remove hospital square from unassigned squares
	auto it = std::find(unassignedSquaresList.begin(), unassignedSquaresList.end(), originHospitalSquare);
	if (it != unassignedSquaresList.end())
		unassignedSquaresList.erase(it);


	// Perform attempts to create large map blocks
	while (largeBlocksPlacementAttempts > 0 && !unassignedSquaresList.empty())
	{
		// Choose first unassigned square
		Vector2i resultSquare = unassignedSquaresList.back();

		// Randomly choose size of the block to be generated (without STANDARD size)
		std::uniform_int_distribution<int> randomSizeEnumDistribution(0, static_cast<int>(Size::SIZE_COUNT) - 2);
		int sizeEnumIndex = randomSizeEnumDistribution(randomGenerator);
		Size resultSize = static_cast<Size>(sizeEnumIndex);

		// Randomly choose area type of the block to be generated
		AreaType resultAreaType = GetRandomAreaType(randomGenerator);

		// Form the block and add it to the list of all map blocks
		std::vector<Vector2i> squaresToFormBlock = FormFullBlock(resultSquare, resultSize, &unassignedSquaresList);
		if (!squaresToFormBlock.empty())
		{
			unassignedSquaresList.pop_back();
			MapBlock newMapBlock(squaresToFormBlock, resultSize, resultAreaType);
			mapBlocksList.push_back(newMapBlock);
		}

		largeBlocksPlacementAttempts--;
	}

	// Create standard map blocks with the rest of unassigned squares and add them to the list of all map blocks
	if (!unassignedSquaresList.empty())
	{
		for (Vector2i unassignedSquare : unassignedSquaresList)
		{
			AreaType resultAreaType = GetRandomAreaType(randomGenerator);
			MapBlock newMapBlock({ unassignedSquare }, Size::STANDARD, resultAreaType);
			mapBlocksList.push_back(newMapBlock);
		}
		unassignedSquaresList.clear();
	}
}

// Create a list of buildings with their types and positions
void Map::GenerateBuildings()
{
	// x and y coordinates of the point where main city rectangle starts to be drawn (top left corner)
	int citySquareOriginX = -mapPixelSize / 2;
	int citySquareOriginY = -mapPixelSize / 2;

	buildingsList.clear();

	// Generate buildings in the loop for each map block
	for (const MapBlock& block : mapBlocksList)
	{
		for (const Vector2i& square : block.GetOccupiedSquares())
		{
			// Get the origin of base occupied square
			int baseBlockOriginX = citySquareOriginX + ROAD_WIDTH + square.x * (SQUARE_WIDTH + ROAD_WIDTH);
			int baseBlockOriginY = citySquareOriginY + ROAD_WIDTH + square.y * (SQUARE_WIDTH + ROAD_WIDTH);

			// Determine the type of inheriting class
			switch (block.GetAreaType())
			{
			case AreaType::RESIDENTIAL_AREA:
				buildingsList.push_back(std::make_unique<House>(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH, &houseTexture));
				break;
			case AreaType::HOSPITAL:
				buildingsList.push_back(std::make_unique<Hospital>(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH, &hospitalTexture));
				break;
			case AreaType::SHOPPING_AREA:
				buildingsList.push_back(std::make_unique<Shop>(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH, &shopTexture));
				break;
			case AreaType::WORKPLACE_AREA:
				buildingsList.push_back(std::make_unique<Workplace>(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH, &workplaceTexture));
				break;
			default:
				continue;
			}
		}
	}
}

// Draw the map blocks on the screen
void Map::DrawMap()
{
	// x and y coordinates of the point where main city rectangle starts to be drawn (top left corner)
	int citySquareOriginX = -mapPixelSize / 2;
	int citySquareOriginY = -mapPixelSize / 2;

	// Draw main city rectangle
	BACKGROUND_COLOR.DrawRectangle(citySquareOriginX, citySquareOriginY, mapPixelSize, mapPixelSize);

	// Draw individual blocks
	for (MapBlock currentMapBlock : mapBlocksList)
	{
		// Get the origin of base occupied square
		std::vector<Vector2i> occupiedSquares = currentMapBlock.GetOccupiedSquares();
		Vector2i occupiedSquare = occupiedSquares.at(0);
		int baseBlockOriginX = citySquareOriginX + ROAD_WIDTH + occupiedSquare.x * (SQUARE_WIDTH + ROAD_WIDTH);
		int baseBlockOriginY = citySquareOriginY + ROAD_WIDTH + occupiedSquare.y * (SQUARE_WIDTH + ROAD_WIDTH);

		// Determine the color of the block and the type of building
		raylib::Color drawColor;

		switch (currentMapBlock.GetAreaType())
		{
		case AreaType::GREEN_AREA:
			drawColor = GREEN_AREA_COLOR;
			break;
		case AreaType::RESIDENTIAL_AREA:
			drawColor = RESIDENTIAL_AREA_COLOR;
			break;
		case AreaType::HOSPITAL:
			drawColor = HOSPITAL_AREA_COLOR;
			break;
		case AreaType::SHOPPING_AREA:
			drawColor = SHOPPING_AREA_COLOR;
			break;
		case AreaType::WORKPLACE_AREA:
			drawColor = WORKPLACE_AREA_COLOR;
			break;
		default:
			// EXCEPTION MISSING
			continue;
		}

		// Draw the block based on its size
		switch (currentMapBlock.GetBlockSize())
		{
		case Size::STANDARD:
			drawColor.DrawRectangle(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH, SQUARE_WIDTH);
			break;

		case Size::DOUBLE_VERTICAL:
			drawColor.DrawRectangle(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH, SQUARE_WIDTH * 2 + ROAD_WIDTH);
			break;

		case Size::DOUBLE_HORIZONTAL:
			drawColor.DrawRectangle(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH * 2 + ROAD_WIDTH, SQUARE_WIDTH);
			break;

		case Size::QUAD_SQUARE:
			drawColor.DrawRectangle(baseBlockOriginX, baseBlockOriginY, SQUARE_WIDTH * 2 + ROAD_WIDTH, SQUARE_WIDTH * 2 + ROAD_WIDTH);
			break;

		default:
			// EXCEPTION MISSING
			continue;
		}
	}
}

// Draw all buildings on the map
void Map::DrawBuildings()
{
	for (const auto& building : buildingsList)
	{
		building->DrawBuilding(SQUARE_WIDTH);
	}
}

Vector2i Map::PixelToGridPosition(Vector2i pixelPosition)
{
	Vector2i relativePosition = { pixelPosition.x + mapPixelSize / 2, pixelPosition.y + mapPixelSize / 2 };

	return {
		static_cast<int>(relativePosition.x / (SQUARE_WIDTH + ROAD_WIDTH)),
		static_cast<int>(relativePosition.y / (SQUARE_WIDTH + ROAD_WIDTH))
	};
}

Vector2i Map::GridToPixelPosition(Vector2i gridPosition)
{
	Vector2i relativePosition{
		gridPosition.x * (SQUARE_WIDTH + ROAD_WIDTH) + ROAD_WIDTH / 2,
		gridPosition.y * (SQUARE_WIDTH + ROAD_WIDTH) + ROAD_WIDTH / 2
	};
	return {
		relativePosition.x - mapPixelSize / 2,
		relativePosition.y - mapPixelSize / 2
	};
}