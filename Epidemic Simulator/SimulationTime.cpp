#include "SimulationTime.h"
#include <iostream>

void SimulationTime::AdvanceTime(float deltaTime)
{
	static float timeAccumulator = 0.0f;
	timeAccumulator += deltaTime;
	if (timeAccumulator >= hourLength)
	{
		hour = (hour + 1) % 24;
		if (hour == 0)
			day++;

		timeAccumulator = 0.0f;
		hourChanged = true;
	}
}

bool SimulationTime::HasHourChanged()
{
	if (hourChanged)
	{
		hourChanged = false;
		return true;
	}
	else return false;
}

void SimulationTime::ChangeHourLength(float newTime) {
	hourLength = newTime;
}