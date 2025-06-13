#pragma once
#include "raylib-cpp.hpp"
#include "Vector2i.h"
#include <iostream>

class Building
{
protected:
	Vector2i originPosition;
	Vector2i position;		// the position of the building at the center
	raylib::Texture2D* texture;

public:
	Building(int x, int y, int squareWidth, raylib::Texture2D* texture) : originPosition(x, y), texture(texture), position({ x + squareWidth / 2, y + squareWidth / 2 }) {}
	virtual void DrawBuilding(int squareWidth);
	virtual ~Building() = default;
	Vector2i GetPosition() const { return position; }
};


class House : public Building
{
private:
	//residents list

public:
	House(int x, int y, int squareWidth, raylib::Texture2D* texture) : Building(x, y, squareWidth, texture) {}
};


class Shop : public Building
{
public:
	Shop(int x, int y, int squareWidth, raylib::Texture2D* texture) : Building(x, y, squareWidth, texture) {}
};


class Hospital : public Building
{
public:
	Hospital(int x, int y, int squareWidth, raylib::Texture2D* texture) : Building(x, y, squareWidth, texture) {}
};


class Workplace : public Building
{
private:
	// employees list

public:
	Workplace(int x, int y, int squareWidth, raylib::Texture2D* texture) : Building(x, y, squareWidth, texture) {}
};