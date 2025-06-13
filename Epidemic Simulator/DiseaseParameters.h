#pragma once

struct DiseaseParameters
{
    float infectionProbabilityPerHour;
    float deathProbabilityPerHour;
    float deathProbabilityPerHourInHospital;
    float probabilityOfGoingToHospitalPerHour;
    float hoursToGetImmune;
    float hoursToGetSymptoms;
    float infectionRadius;
};