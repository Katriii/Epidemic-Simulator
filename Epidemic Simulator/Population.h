#pragma once
#include "Person.h"
#include "SimulationTime.h"
#include <vector>
#include "DiseaseParameters.h"

const int RESIDENTS_IN_BUILDING_LIMIT = 5;
const int INITIAL_IMMUNE_PERCENTAGE = 5; // Percentage of people that are immune at the start of the simulation

class SimulationTime;

class Population
{
private:
	std::vector<Person> peopleList;
	Map* map;
	Building* hospitalBuilding;
	DiseaseParameters diseaseParameters;
	int residentsInBuildingLimit;
	static inline float populationCount;

public:
	Population(int personCount, Map* map, const DiseaseParameters& parameters, int residentsInBuildingLimit); // Constructor to initialize the population with a given number of people
	void UpdatePopulationOnHour(int currentHour);
	void UpdatePopulationOnFrame(float deltaTime);
	void UpdateSimulationSpeed(float hourLength);

	void DrawPopulation() const;
	int GetHealthyCount() const;
	int GetInfectedCount() const;
	int GetImmuneCount() const;
	int GetDeadCount() const;
	void ChangePopulationParameters(DiseaseParameters* newDiseaseParameters);

	friend class DataColumn;
};