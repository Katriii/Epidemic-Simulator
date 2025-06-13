#pragma once

#include "raylib-cpp.hpp"
#include "Population.h"
#include <iostream>
#include <string>
#include <vector>

// 1-pixel wide column of population data visible on the graph
class DataColumn {
private:
	float posX{}, posY{};
	float width = 1, height{};
	raylib::Rectangle healthy;
	raylib::Rectangle infected;
	raylib::Rectangle immune;
	raylib::Rectangle dead;
public:
	DataColumn(float x, float y, float w, float h, Population* population);
	virtual ~DataColumn() {};

	void drawDataColumn();
	friend class Graph;
};

// hour count displayed on the x-axis
class TimeUnit {
private:
	float posX{}, posY{};
	float width = 5, height = 10;
	int hourCount;
public:
	TimeUnit(float x, float y, float w, float h, int hour);
	virtual ~TimeUnit() {};

	void drawTimeUnit();
	friend class Graph;
};

class Graph {
private:
	float posX{}, posY{};
	float width{}, height{};
	float axisWidth = 5;
	raylib::Rectangle xAxisLine;
	raylib::Rectangle yAxisLineLeft;
	raylib::Rectangle yAxisLineRight;
	std::vector<DataColumn> columnCollection;
	std::vector<TimeUnit> unitCollection;
public:
	Graph(float x, float y, float w, float h);
	virtual ~Graph() {};

	float getPosX();
	float getPosY();
	float getWidth();
	float getHeight();
	float getAxisWidth();

	void drawGraph();
	void updateGraphStart(int* filler, float simulationTime, Population* population); // initial fill-in
	void updateGraph(int* framecounter, float simulationTime, Population* population); // normal graph updates
};