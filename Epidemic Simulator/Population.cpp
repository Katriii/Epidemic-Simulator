#include "raylib-cpp.hpp"
#include "Population.h"
#include <random>
#include <unordered_map>
#include <iostream>

// Initialize population assigning every person a house and a workplace
Population::Population(int personCount, Map* map, const DiseaseParameters& parameters, int residentsInBuildingLimit) : map(map), diseaseParameters(parameters), residentsInBuildingLimit(residentsInBuildingLimit), hospitalBuilding(nullptr)
{
    populationCount = personCount;
    // Get pointers to residential and workplace buildings
    std::vector<Building*> residentialBuildings;
    std::vector<Building*> workplaceBuildings;
    std::vector<Building*> shoppingBuildings;

    // Fill residential and workplace buildings lists based on general buildings list
    for (const auto& building : map->GetBuildingsList())
    {
        if (dynamic_cast<House*>(building.get()))
            residentialBuildings.push_back(building.get());

        if (dynamic_cast<Workplace*>(building.get()))
            workplaceBuildings.push_back(building.get());

        if (dynamic_cast<Shop*>(building.get()))
            shoppingBuildings.push_back(building.get());

        if (dynamic_cast<Hospital*>(building.get()))
            hospitalBuilding = building.get();
    }

    // Initialize each building's residents count
    std::unordered_map<Building*, int> residentsCount;
    for (auto* building : residentialBuildings)
    {
        residentsCount[building] = 0;
    }

    // Initialize RNG
    std::random_device randomDevice;
    std::mt19937 gen(randomDevice());

    // Create people and assign them to buildings
    for (int i = 0; i < personCount; ++i)
    {
        // Create a list of buildings that can accommodate this person
        std::vector<Building*> availableHouses;
        for (auto* house : residentialBuildings)
        {
            if (residentsCount[house] < residentsInBuildingLimit)
                availableHouses.push_back(house);
        }

        // Stop generating people if no buildings are available
        if (availableHouses.empty()) {
            throw std::runtime_error("No residential buildings are available for the population");
        }

        // Randomly select a house from the available ones
        std::uniform_int_distribution<> houseDistribution(0, (int)availableHouses.size() - 1);
        Building* selectedHouse = availableHouses[houseDistribution(gen)];
        residentsCount[selectedHouse]++;

        // Randomly select a workplace for the person
        if (workplaceBuildings.empty()) {
            throw std::runtime_error("No workplace buildings are available for the population");
        }
        std::uniform_int_distribution<> workplaceDistribution(0, (int)workplaceBuildings.size() - 1);
        Building* selectedWorkplace = workplaceBuildings[workplaceDistribution(gen)];

        // Randomly select a shop for the person
        if (shoppingBuildings.empty()) {
            throw std::runtime_error("No shopping buildings are available for the population");
        }
        std::uniform_int_distribution<> shopDistribution(0, (int)shoppingBuildings.size() - 1);
        Building* selectedShop = shoppingBuildings[shopDistribution(gen)];

        if (!hospitalBuilding) {
            throw std::runtime_error("No hospital building available for the population");
        }

        // Set the initial position of the person to the house's position
        Vector2i initialPosition(selectedHouse->GetPosition());

        // Select the state of the person
        PersonState newState;
        if (i == 0) {
            newState = Infected; // First person is infected
        }
        else {
            // 10% chance of being immune, otherwise healthy
            newState = (GetRandomValue(0, 100) < INITIAL_IMMUNE_PERCENTAGE) ? Immune : Healthy;
        }

        // Create a new person and add it to the population
        Person newPerson(initialPosition, newState, selectedHouse, selectedWorkplace, selectedShop, hospitalBuilding, map, diseaseParameters);
        peopleList.push_back(newPerson);
    }
}

void Population::UpdatePopulationOnHour(int currentHour)
{
    for (size_t i = 0; i < peopleList.size(); ++i) {
        peopleList[i].UpdatePersonOnHour(currentHour);
    }
    std::cout << "Healthy: " << GetHealthyCount() << ", Infected: " << GetInfectedCount()
        << ", Immune: " << GetImmuneCount() << ", Dead: " << GetDeadCount() << std::endl;
}

void Population::UpdatePopulationOnFrame(float deltaTime) {
    for (size_t i = 0; i < peopleList.size(); ++i)
    {
        peopleList[i].UpdatePersonOnFrame(deltaTime);

        // Perform infections
        if (peopleList[i].GetState() == Infected && !peopleList[i].IsInHospital())
        {
            for (size_t j = 0; j < peopleList.size(); ++j)
            {
                if (i != j && peopleList[j].GetState() == Healthy && peopleList[i].CheckCollision(peopleList[j]))
                    peopleList[j].TryToGetInfected();
            }
        }
    }
}

void Population::UpdateSimulationSpeed(float hourLength)
{
    for (size_t i = 0; i < peopleList.size(); ++i) {
        peopleList[i].UpdateSimulationSpeed(hourLength);
    }
}

void Population::DrawPopulation() const {
    for (size_t i = 0; i < peopleList.size(); ++i) {
        peopleList[i].DrawPerson();
    }
}

int Population::GetHealthyCount() const {
    int count = 0;
    for (const Person& person : peopleList) {
        if (person.GetState() == Healthy) {
            count++;
        }
    }
    return count;
}

int Population::GetInfectedCount() const {
    int count = 0;
    for (const Person& person : peopleList) {
        if (person.GetState() == Infected) {
            count++;
        }
    }
    return count;
}

int Population::GetImmuneCount() const {
    int count = 0;
    for (const Person& person : peopleList) {
        if (person.GetState() == Immune) {
            count++;
        }
    }
    return count;
}

int Population::GetDeadCount() const {
    int count = 0;
    for (const Person& person : peopleList) {
        if (person.GetState() == Dead) {
            count++;
        }
    }
    return count;
}

void Population::ChangePopulationParameters(DiseaseParameters* newDiseaseParameters) {
    //float deathProbabilityPerHour;
    //float hoursToGetImmune;
    //float hoursToGetSymptoms;

    for (int i = 0; i < peopleList.size(); i++) {
        peopleList[i].ChangeDiseaseParameters(newDiseaseParameters);
    }
}