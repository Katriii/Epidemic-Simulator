#pragma once

class SimulationTime
{
private:
	float hourLength; // Length of the simulation's hour in seconds
	int hour;
	int day;
	bool hourChanged;

public:
	SimulationTime(float hourLengthInSeconds = 1.0f) : hour(0), day(1), hourLength(hourLengthInSeconds), hourChanged(false) {}
	void AdvanceTime(float deltaTime);
	int GetHour() const { return hour; }
	int GetDay() const { return day; }
	float GetHourLength() const { return hourLength; }
	bool HasHourChanged();
	void ChangeHourLength(float newTime);
};