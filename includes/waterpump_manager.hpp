#ifndef WATER_PUMP_MANAGER_H
#define WATER_PUMP_MANAGER_H

#include <memory>
#include <Arduino.h>
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
  void calculateWaterLevel();
  bool shouldWaterPumpStart();
  void startPump(byte relayPin);
  void checkWaterLevel();
  void stopPump();
  double getDistance(int numberOfRepeats = 10);

private:
  double getValidDistance(std::vector<double> const &distances, double errorRange);
  // If after the maxTries stills returns 0 cm, it will return BAD_SENSOR_LECTURE indicating an error
  double getDistanceOrReturnErrorAfterMaxTries(int maxTries = 100);
  double getDistanceInternal();
  void handleWaterInsufficiency();
  int getPassedSeconds();
  double getFirstDistance();
};

#endif
