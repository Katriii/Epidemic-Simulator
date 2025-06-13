#pragma once
#include "raylib-cpp.hpp"
#include "Map.h"
#include "Building.h"
#include "SimulationTime.h"
#include "Vector2i.h"
#include "DiseaseParameters.h"

const float DRAW_RADIUS = 10.0f; // Radius for drawing the person
const raylib::Color HEALTHY_COLOR = GREEN;
const raylib::Color INFECTED_COLOR = RED;
const raylib::Color IMMUNE_COLOR = SKYBLUE;
const raylib::Color DEAD_COLOR = BLACK;

class SimulationTime;

enum PersonState
{
	Healthy,
	Infected,
	Immune,
	Dead
};

struct PersonSchedule
{
	int workStartHour;
	int workEndHour;
	int shoppingStartHour;
	int shoppingEndHour;
};


class Person
{
private:
	// Position of the person in the simulation
	Vector2i position;
	Vector2i gridPosition;

	// Parameters related to simulation speed
	float hourLength; // Length of an hour in seconds
	float movementSpeed; // Speed of the person calculated every frame based on an hour length and square width
	float infectionProbabilityPerFrame;
	float deathProbabilityPerFrame;
	float deathProbabilityPerFrameInHospital;
	float probabilityOfGoingToHospital;
	float framesPerHour;

	// Health state of the person
	PersonState state;	// Current state of the person (healthy, infected, immune, dead)
	float timeSinceInfected;	// Time since the person was infected

	// Global parameters
	DiseaseParameters diseaseParameters;
	Map* map;
	Building* hospitalBuilding;

	// Parameters specific to the person
	Building* house; // The building where the person lives
	Building* workplaceBuilding; // The building where the person works
	Building* shoppingBuilding; // The building where the person shops
	Building* currentBuilding; // The building the person is currently in
	PersonSchedule schedule; // Daily schedule of the person

	// Variables for pathfinding
	Vector2i currentIntersection; // The current intersection the person is at
	Vector2i nextIntersection; // The next intersection the person is moving towards
	Vector2i nextIntersectionPixel;
	Vector2i targetIntersection; // The target intersection the person is moving towards
	bool reachedDestination;

public:
	Person(Vector2i initialPosition, PersonState initialState, Building* assignedHouse, Building* assignedWorkplaceBuilding, Building* assignedShoppingBuilding, Building* hospital, Map* map, const DiseaseParameters& parameters);
	void UpdatePersonOnHour(int currentHour);	// Update the person's current building every hour
	void UpdatePersonOnFrame(float deltaTime);	// Update the person's health state, position and movement speed every frame
	void UpdateSimulationSpeed(float hourLength);
	void MoveTowardsCurrentBuilding(float deltaTime);
	Vector2i GetNextIntersection(Vector2i& currentIntersection, Vector2i& targetIntersection);
	void PrepareToMoveToBuilding(Building* newBuilding);
	void DrawPerson() const;
	void ChangeDiseaseParameters(DiseaseParameters* diseaseParameters);

	Vector2i GetPosition() const { return position; }
	bool IsInHospital() const { return currentBuilding == hospitalBuilding; }
	PersonState GetState() const { return state; }

	void TryToGetInfected();
	void TryToDie(float deathProbability);
	void TryToGoToHospital();
	bool IsAlive() const;
	bool CheckCollision(const Person& other) const;
};