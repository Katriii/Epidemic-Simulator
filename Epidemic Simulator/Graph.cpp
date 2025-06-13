#include "Graph.h"
#include "Population.h"
// parametry konstruktora klasy Rectangle:
// raylib::Rectangle(float x, float y, float width, float height)
// 
//-------------------- DataColumn ----------------------------
DataColumn::DataColumn(float x, float y, float w, float h, Population* population) {
	posX = x;
	posY = y;
	width = w;
	height = h;

	// update simulation parameters here
	float RatioHealthy = population->GetHealthyCount() / Population::populationCount;
	float RatioInfected = population->GetInfectedCount() / Population::populationCount;
	float RatioImmune = population->GetImmuneCount() / Population::populationCount;
	float RatioDead = population->GetDeadCount() / Population::populationCount;

	healthy = { posX, posY, width, height * RatioHealthy };
	immune = { posX, posY + height * RatioHealthy, width, height * RatioImmune };
	infected = { posX, posY + height * (RatioHealthy + RatioImmune), width, height * RatioInfected};
	dead = { posX, posY + height * (RatioHealthy + RatioImmune + RatioInfected), width, height * RatioDead };
}

void DataColumn::drawDataColumn() {
	healthy.Draw(raylib::Color::Green());
	immune.Draw(raylib::Color::Blue());
	infected.Draw(raylib::Color::Red());
	dead.Draw(raylib::Color::Black());
}

//-------------------- TimeUnit ----------------------------
TimeUnit::TimeUnit(float x, float y, float w, float h, int hour) {
	posX = x;
	posY = y;
	width = w;
	height = h;
	hourCount = hour;
}

void TimeUnit::drawTimeUnit() {
	raylib::Color::Black().DrawRectangle(posX, posY, width, height);
	raylib::Color::Black().DrawText(std::to_string(hourCount) + ":00", posX - 10 - 7, posY + 15, 19);
}

//-------------------- Graph ----------------------------
Graph::Graph(float x, float y, float w, float h) {
	posX = x;
	posY = y;
	width = w;
	height = h;

	xAxisLine = { posX, posY + height, width, axisWidth}; 
	yAxisLineLeft = { posX, posY, axisWidth, height };
	yAxisLineRight = { posX + width, posY, axisWidth, height };
}

float Graph::getPosX() { return posX; }
float Graph::getPosY() { return posY; }
float Graph::getWidth() { return width; }
float Graph::getHeight() { return height; }
float Graph::getAxisWidth() { return axisWidth; }

void Graph::drawGraph() {
	xAxisLine.Draw(raylib::Color::Black());
	yAxisLineLeft.Draw(raylib::Color::Black());
	yAxisLineRight.Draw(raylib::Color::Black());
	
	// y-axis: 
	for (int i = posY; i <= posY + height; i += height/10) {
		int percentageY = 100 - (((i - posY) * 100 )/ height);
		// left side
		raylib::Color::Black().DrawRectangle(posX - 10, i, 15, axisWidth);
		raylib::Color::Black().DrawText(std::to_string(percentageY) + "%", posX - 47, i - 6, 19);
		// right side
		raylib::Color::Black().DrawRectangle(posX + width, i, 15, axisWidth);
		raylib::Color::Black().DrawText(std::to_string(percentageY) + "%", posX + width + axisWidth + 15, i - 6, 19);
	}

	// collumnCollection
	for (int i = 0; i < columnCollection.size(); i++) {
		columnCollection[i].drawDataColumn();
	} 

	// timeUnitCollection
	for (int i = 0; i < unitCollection.size(); i++) {
		unitCollection[i].drawTimeUnit();
	}
}

// phase 1 - initial graph fill-in:
void Graph::updateGraphStart(int* filler, float simulationTime, Population* population) {
	int simulationMinutes = trunc(ceil(simulationTime * 60.f));
	// columns:
	columnCollection.push_back(DataColumn(posX + 5 + *filler, posY, 1, height, population));
	
	// time units:
	if (*filler % simulationMinutes == 0) {
		unitCollection.push_back(TimeUnit(posX + *filler, posY + height + axisWidth,
		5, 10, (*filler/simulationMinutes) ));
	}

	*filler += 1;
}

// phase 2 - graph scrolling
void Graph::updateGraph(int* framecounter, float simulationTime, Population* population) {
	int simulationMinutes = trunc(ceil(simulationTime * 60.f));
	
	// Columns:
	// delete the first data column
	columnCollection.erase(columnCollection.begin());
	// re-align all the other columns to the left
	for (int i = 0; i < columnCollection.size(); i++) {
		columnCollection[i].healthy.x -= 1;
		columnCollection[i].infected.x -= 1;
		columnCollection[i].immune.x -= 1;
		columnCollection[i].dead.x -= 1;
	}
	// draw the last data column with updated simulation parameters (done inside the column constructor)
	columnCollection.push_back(DataColumn(posX + width - 1, posY, 1, height, population));

	// Time units:
	// delete all timeunits outside the graph range
	if (unitCollection[0].posX < (posX + 5)) {
		unitCollection.erase(unitCollection.begin());
	}
	// move every timeUnit a pixel to the left
	for (int i = 0; i < unitCollection.size(); i++) {
		unitCollection[i].posX -= 1;
	}
	// draw a new timeUnit every simulation tick (1 hour = 1 second)
	if (*framecounter % simulationMinutes == 0) {
		unitCollection.push_back(TimeUnit(posX + width - 5, posY + height + axisWidth,
			5, 10, *framecounter / simulationMinutes));
	}
}
