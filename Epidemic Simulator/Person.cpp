#include "Person.h"
#include <random>

Person::Person(Vector2i initialPosition, PersonState initialState, Building* assignedHouse, Building* assignedWorkplaceBuilding, Building* assignedShoppingBuilding, Building* hospital, Map* map, const DiseaseParameters& parameters) :
    position(initialPosition),
    state(initialState),
    house(assignedHouse),
    workplaceBuilding(assignedWorkplaceBuilding),
    shoppingBuilding(assignedShoppingBuilding),
    currentBuilding(nullptr),
    timeSinceInfected(0.0f),
    movementSpeed(0.0f),
    schedule({ 0, 0, 0 }),
    map(map),
    reachedDestination(true),
    diseaseParameters(parameters),
    deathProbabilityPerFrameInHospital(0.0f),
    deathProbabilityPerFrame(0.0f),
    framesPerHour(0.0f),
    hospitalBuilding(hospital),
    hourLength(0.0f),
    infectionProbabilityPerFrame(0.0f),
    probabilityOfGoingToHospital(parameters.probabilityOfGoingToHospitalPerHour)
{
    // Initialize a schedule for the person
    int workStart = GetRandomValue(5, 8);   // Work starts between 5 AM and 8 AM
    int workEnd = (workStart + 9) % 24;     // Work lasts for 9 hours
    int shoppingStart = GetRandomValue(workEnd + 1, workEnd + 3);   // Shopping starts after work ends, between 1 and 3 hours later
    int shoppingEnd = (GetRandomValue(shoppingStart + 1, shoppingStart + 2)) % 24;  // Shopping lasts for 1 or 2 hours

    schedule.workStartHour = workStart;
    schedule.workEndHour = workEnd;
    schedule.shoppingStartHour = shoppingStart;
    schedule.shoppingEndHour = shoppingEnd;
}

void Person::UpdatePersonOnHour(int currentHour)
{
    if (!IsInHospital())
    {
        if (currentHour == schedule.workStartHour)
        {
            PrepareToMoveToBuilding(workplaceBuilding);
        }
        else if (currentHour == schedule.workEndHour)
        {
            PrepareToMoveToBuilding(house);
        }
        else if (currentHour == schedule.shoppingStartHour)
        {
            if (reachedDestination)
                PrepareToMoveToBuilding(shoppingBuilding);
        }
        else if (currentHour == schedule.shoppingEndHour)
        {
            PrepareToMoveToBuilding(house);
        }
    }
}

void Person::UpdatePersonOnFrame(float deltaTime)
{
    // Update person's health state
    if (state == Infected)
    {
        timeSinceInfected += deltaTime / hourLength;

        // Check if the person becomes immune
        if (timeSinceInfected > diseaseParameters.hoursToGetImmune)
        {
            state = Immune;
            if (IsInHospital())
                PrepareToMoveToBuilding(house);
        }


        // When the person gets symptoms, they have a chance to die or go to the hospital
        if (timeSinceInfected > diseaseParameters.hoursToGetSymptoms)
        {
            currentBuilding == hospitalBuilding ? TryToDie(deathProbabilityPerFrameInHospital) : TryToDie(deathProbabilityPerFrame);
            TryToGoToHospital();
        }
    }

    if (state == Dead)
        return;

    MoveTowardsCurrentBuilding(deltaTime);
}

// Adjui the simulation speed based on the hour length
void Person::UpdateSimulationSpeed(float hourLength)
{
    this->hourLength = hourLength;
    framesPerHour = 60.0f * hourLength;

    movementSpeed = (10.0f * map->GetSquareWidth()) / hourLength;
    infectionProbabilityPerFrame = 1.0f - pow(1.0f - diseaseParameters.infectionProbabilityPerHour, 1.0f / framesPerHour);
    deathProbabilityPerFrame = 1.0f - pow(1.0f - diseaseParameters.deathProbabilityPerHour, 1.0f / framesPerHour);
    deathProbabilityPerFrameInHospital = 1.0f - pow(1.0f - diseaseParameters.deathProbabilityPerHourInHospital, 1.0f / framesPerHour);
    probabilityOfGoingToHospital = 1.0f - pow(1.0f - diseaseParameters.probabilityOfGoingToHospitalPerHour, 1.0f / framesPerHour);
}

void Person::PrepareToMoveToBuilding(Building* newBuilding)
{
    currentBuilding = newBuilding;
    reachedDestination = false;

    currentIntersection = map->PixelToGridPosition(position);
    targetIntersection = map->PixelToGridPosition(currentBuilding->GetPosition());

    nextIntersection = GetNextIntersection(currentIntersection, targetIntersection);
    nextIntersectionPixel = map->GridToPixelPosition(nextIntersection);
}

void Person::MoveTowardsCurrentBuilding(float deltaTime)
{
    if (!reachedDestination)
    {
        if (!currentBuilding) return;

        // If the person is on the intersection close to the target building, set the position directly
        if (currentIntersection == targetIntersection)
        {
            position = currentBuilding->GetPosition();
            reachedDestination = true;
            return;
        }

        // If the person reached the next intersection, update to the next one
        if (position == nextIntersectionPixel)
        {
            currentIntersection = nextIntersection;
            nextIntersection = GetNextIntersection(currentIntersection, targetIntersection);
            nextIntersectionPixel = map->GridToPixelPosition(nextIntersection);
        }

        if (position.x != nextIntersectionPixel.x) {
            int dir = (nextIntersectionPixel.x > position.x) ? 1 : -1;
            int step = static_cast<int>(movementSpeed * deltaTime);
            int newX = position.x + dir * step;

            if ((dir > 0 && newX > nextIntersectionPixel.x) || (dir < 0 && newX < nextIntersectionPixel.x))
                newX = nextIntersectionPixel.x;
            position.x = newX;
        }
        else if (position.y != nextIntersectionPixel.y) {
            int dir = (nextIntersectionPixel.y > position.y) ? 1 : -1;
            int step = static_cast<int>(movementSpeed * deltaTime);
            int newY = position.y + dir * step;
            if ((dir > 0 && newY > nextIntersectionPixel.y) || (dir < 0 && newY < nextIntersectionPixel.y))
                newY = nextIntersectionPixel.y;
            position.y = newY;
        }
    }
}

Vector2i Person::GetNextIntersection(Vector2i& currentIntersection, Vector2i& targetIntersection)
{
    Vector2i newIntersection = currentIntersection;
    if (currentIntersection.x != targetIntersection.x)
    {
        if (currentIntersection.x < targetIntersection.x)
            newIntersection.x++;
        else if (currentIntersection.x > targetIntersection.x)
            newIntersection.x--;
        return newIntersection;
    }

    else if (currentIntersection.y != targetIntersection.y)
    {
        if (currentIntersection.y < targetIntersection.y)
            newIntersection.y++;
        else if (currentIntersection.y > targetIntersection.y)
            newIntersection.y--;
        return newIntersection;
    }
}

void Person::DrawPerson() const
{
    raylib::Color color;

    switch (state) {
    case Healthy:
        color = HEALTHY_COLOR;
        break;
    case Infected:
        color = INFECTED_COLOR;
        break;
    case Immune:
        color = IMMUNE_COLOR;
        break;
    case Dead:
        color = DEAD_COLOR;
        break;
    }
    DrawCircleV(position.Vector2iToVector2(), DRAW_RADIUS, color);
}

void Person::TryToGetInfected()
{
    if ((float)GetRandomValue(0, 9999) / 10000.0f < infectionProbabilityPerFrame && !IsInHospital())
    {
        state = Infected;
        timeSinceInfected = 0.0f;
    }
}

void Person::TryToDie(float deathProbability)
{
    if ((float)GetRandomValue(0, 9999) / 10000.0f < deathProbability)
    {
        state = Dead;
    }
}

void Person::TryToGoToHospital()
{
    if ((float)GetRandomValue(0, 9999) / 10000.0f < probabilityOfGoingToHospital)
    {
        PrepareToMoveToBuilding(hospitalBuilding);
    }
}

bool Person::IsAlive() const
{
    return state != Dead;
}

bool Person::CheckCollision(const Person& other) const
{
    int distance = position.DistanceTo(other.position);
    return distance < diseaseParameters.infectionRadius * 2;
}

void Person::ChangeDiseaseParameters(DiseaseParameters* newDiseaseParameters) {
    diseaseParameters.infectionProbabilityPerHour = newDiseaseParameters->infectionProbabilityPerHour;
    diseaseParameters.deathProbabilityPerHour = newDiseaseParameters->deathProbabilityPerHour;
    diseaseParameters.hoursToGetImmune = newDiseaseParameters->hoursToGetImmune;
    diseaseParameters.hoursToGetSymptoms = 0.5f * newDiseaseParameters->hoursToGetSymptoms;
    diseaseParameters.deathProbabilityPerHourInHospital = 0.5f * newDiseaseParameters->deathProbabilityPerHour;
 
    //diseaseParameters.infectionProbabilityPerHour = newDiseaseParameters->infectionProbabilityPerHour;
    //float deathProbabilityPerHour;
    //float hoursToGetImmune;
    //float hoursToGetSymptoms;
}