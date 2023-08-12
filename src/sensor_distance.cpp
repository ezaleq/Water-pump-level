#include <vector>
#include "../includes/waterpump_manager.hpp"
#define BAD_SENSOR_LECTURE -1


static double getAverage(std::vector<double> const &values)
{
    auto sum = 0.0f;
    for (auto i = 0; i < values.size(); i++)
    {
        sum += values[i];
    }
    return sum / values.size();
}

void WaterPumpManager::calculateWaterLevel()
{
    auto distance = this->getDistance();
    if (distance == BAD_SENSOR_LECTURE)
    {
        this->handleWaterInsufficiency();
        return;
    }

    WaterPumpManager::distanceInCm = distance;
    WaterPumpManager::waterLevel = 100 - (distance * 100) / config->waterTankHeight;
    if (WaterPumpManager::waterLevel > 100)
        this->calculateWaterLevel();
}

double WaterPumpManager::getDistance(int numberOfRepeats)
{
    std::vector<double> distances;
    auto firstDistance = this->getDistanceOrReturnErrorAfterMaxTries();
    if (firstDistance == BAD_SENSOR_LECTURE)
    {
        return BAD_SENSOR_LECTURE;
    }
    distances.push_back(firstDistance);
    for (auto i = 0; i < numberOfRepeats; i++)
    {
        auto validDistance = this->getValidDistance(distances, 5);
        if (validDistance == BAD_SENSOR_LECTURE)
        {
            return BAD_SENSOR_LECTURE;
        }
        distances.push_back(validDistance);
    }
    return getAverage(distances);
}

double WaterPumpManager::getValidDistance(std::vector<double> const &distances, double errorRange)
{
    auto average = getAverage(distances);
    auto min = average - errorRange;
    auto max = average + errorRange;
    auto currentDistance = 0;

    while (currentDistance < min || currentDistance > max)
    {
        currentDistance = this->getDistanceOrReturnErrorAfterMaxTries();
        if (currentDistance == BAD_SENSOR_LECTURE)
            return BAD_SENSOR_LECTURE;
    }
    return currentDistance;
}

double WaterPumpManager::getDistanceOrReturnErrorAfterMaxTries(int maxTries)
{
    for (auto tries = 0; tries < maxTries; tries++)
    {
        auto currentDistance = this->getDistanceInternal();
        if (currentDistance != 0)
            return currentDistance;
    }
    return BAD_SENSOR_LECTURE;
}

double WaterPumpManager::getDistanceInternal()
{
    digitalWrite(this->triggerPin, LOW);
    delayMicroseconds(15);
    digitalWrite(this->triggerPin, HIGH);
    delayMicroseconds(50); // Use a longer pulse width for JSN-SR04T
    digitalWrite(this->triggerPin, LOW);
    auto duration = pulseIn(this->echoPin, HIGH);
    auto distance = duration * 0.034 / 2;
    return distance;
}
