#ifndef WATER_PUMP_MANAGER_H
#define WATER_PUMP_MANAGER_H

#define BAD_SENSOR_LECTURE -1
#include "structs/waterpump_configuration.hpp"

class WaterPumpManager
{

private:
  WaterPumpConfiguration *config;
  unsigned long startTimePump = millis();
  byte triggerPin = 0;
  byte echoPin = 0;
  float startWaterLevel = 0;
  byte relayPin = 0;

public:
  static bool isRunning;
  static bool waterError;
  static float waterLevel;
  static int distanceInCm;

  WaterPumpManager(WaterPumpConfiguration *config, byte triggerPin, byte echoPin)
  {
    this->triggerPin = triggerPin;
    this->echoPin = echoPin;
    this->config = config;
  }

  void calculateWaterLevel()
  {
    auto distance = getDistance();
    if (distance == BAD_SENSOR_LECTURE)
    {
      this->handleWaterInsufficiency();
      return;
    }

    WaterPumpManager::distanceInCm = distance;
    WaterPumpManager::waterLevel = 100 - (distance * 100) / config->waterTankHeight;
    if (WaterPumpManager::waterLevel > 100)
      calculateWaterLevel();
  }
  bool shouldWaterPumpStart()
  {
    return WaterPumpManager::waterLevel <= config->minWaterLevel && config->automaticPump;
  }
  void startPump(byte relayPin)
  {
    waterError = false;
    this->relayPin = relayPin;
    digitalWrite(relayPin, HIGH);
    this->startTimePump = millis();
    this->startWaterLevel = WaterPumpManager::waterLevel;
    isRunning = true;
    this->relayPin = relayPin;
  }
  void checkWaterLevel()
  {
    if (this->getPassedSeconds() > config->timeToDetectWaterInsufficiency && WaterPumpManager::waterLevel <= this->startWaterLevel + 10)
    {
      Serial.println("Error in water");
      this->handleWaterInsufficiency();
    }
    else if (WaterPumpManager::waterLevel >= config->maxWaterLevel)
    {
      stopPump();
    }
    else if (WaterPumpManager::waterLevel > startWaterLevel)
    { // If the waterLevel updated and incremented in level, reset timer
      this->startTimePump = millis();
      startWaterLevel = WaterPumpManager::waterLevel;
    }
  }
  void stopPump()
  {
    isRunning = false;
    digitalWrite(this->relayPin, LOW);
    this->startTimePump = millis();
    this->startWaterLevel = WaterPumpManager::waterLevel;
  }
  double getDistance(int numberOfRepeats = 10)
  {
    std::vector<double> distances;
    auto firstDistance = getDistanceOrReturnErrorAfterMaxTries();
    if (firstDistance == BAD_SENSOR_LECTURE)
    {
      return BAD_SENSOR_LECTURE;
    }
    distances.push_back(firstDistance);
    for (auto i = 0; i < numberOfRepeats; i++)
    {
      auto validDistance = getValidDistance(distances, 5);
      if (validDistance == BAD_SENSOR_LECTURE) {
        return BAD_SENSOR_LECTURE;
      }
      distances.push_back(validDistance);
    }
    return getAverage(distances);
  }

private:
  double getValidDistance(std::vector<double> const &distances, double errorRange)
  {
    auto average = getAverage(distances);
    auto min = average - errorRange;
    auto max = average + errorRange;
    auto currentDistance = 0;

    while(currentDistance < min || currentDistance > max)
    {
      currentDistance = getDistanceOrReturnErrorAfterMaxTries();
      if (currentDistance == BAD_SENSOR_LECTURE)
        return BAD_SENSOR_LECTURE;
    }
    return currentDistance;

  }
  double getAverage(std::vector<double> const &values)
  {
    auto sum = 0.0f;
    for (auto i = 0; i < values.size(); i++)
    {
      sum += values[i];
    }
    return sum / values.size();
  }

  // If after the maxTries stills returns 0 cm, it will return BAD_SENSOR_LECTURE indicating an error
  double getDistanceOrReturnErrorAfterMaxTries(int maxTries = 100)
  {
    for (auto tries = 0; tries < maxTries; tries++)
    {
      auto currentDistance = GetDistanceInternal();
      if (currentDistance != 0)
        return currentDistance;
    }
    return BAD_SENSOR_LECTURE;
  }

  double GetDistanceInternal()
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
  void handleWaterInsufficiency()
  {
    config->automaticPump = false;
    stopPump();
    waterError = true;
  }
  int getPassedSeconds()
  {
    return (millis() - startTimePump) / 1000;
  }
};
float WaterPumpManager::waterLevel = 0;
bool WaterPumpManager::isRunning = false;
bool WaterPumpManager::waterError = false;
int WaterPumpManager::distanceInCm = 0;
#endif
