#pragma once
#include "raylib-cpp.hpp"
#include <iostream>
#include "MapBlock.h"
#include "Building.h"

class Map
{
private:
    const raylib::Color BACKGROUND_COLOR = raylib::Color(85, 85, 85);
    const raylib::Color GREEN_AREA_COLOR = raylib::Color(52, 166, 77);
    const raylib::Color RESIDENTIAL_AREA_COLOR = raylib::Color(164, 164, 164);
    const raylib::Color HOSPITAL_AREA_COLOR = raylib::Color(187, 81, 104);
    const raylib::Color SHOPPING_AREA_COLOR = raylib::Color(224, 154, 93);
    const raylib::Color WORKPLACE_AREA_COLOR = raylib::Color(141, 162, 255);

    raylib::Texture2D houseTexture;
    raylib::Texture2D shopTexture;
    raylib::Texture2D hospitalTexture;
    raylib::Texture2D workplaceTexture;

    const int SQUARE_WIDTH = 100;
    const int ROAD_WIDTH = 30;
    const float LARGE_BLOCKS_PLACEMENT_INTENSITY = 10.0f;

    std::vector<MapBlock> mapBlocksList;
    std::vector<std::unique_ptr<Building>> buildingsList;
    Building* hospitalBuilding = nullptr;
    int mapSquareSize;
    int mapPixelSize;

public:
    Map(int populationSize, int residentsInBuildingLimit);
    void GenerateMap();
    void GenerateBuildings();
    void DrawMap();
    void DrawBuildings();
    Vector2i PixelToGridPosition(Vector2i pixelPosition);
    Vector2i GridToPixelPosition(Vector2i gridPosition);
    const std::vector<std::unique_ptr<Building>>& GetBuildingsList() const { return buildingsList; }
    int GetSquareWidth() const { return SQUARE_WIDTH; }
    int GetMapWidth() const { return mapSquareSize; }
};