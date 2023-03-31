#ifndef WATER_PUMP_MANAGER_H
#define WATER_PUMP_MANAGER_H

#include "structs/WaterPumpConfiguration.h"
#include <NewPing.h>

class WaterPumpManager
{

private:
  WaterPumpConfiguration *config;
  unsigned long startTimePump = millis();
  byte triggerPin = 0;
  byte echoPin = 0;
  float startWaterLevel = 0;
  byte relayPin = 0;
  NewPing* newPing = NULL;

public:
  static bool isRunning;
  static bool waterError;
  static float waterLevel;

  WaterPumpManager(WaterPumpConfiguration *config, byte triggerPin, byte echoPin)
  {
    this->newPing = new NewPing(triggerPin, echoPin);
    this->triggerPin = triggerPin;
    this->echoPin = echoPin;
    this->config = config;
  }

  void CalculateWaterLevel()
  {
    // auto distance = GetDistance(this->triggerPin, this->echoPin);
    auto distance = GetDistance();
    WaterPumpManager::waterLevel = 100 - (distance * 100) / config->waterTankHeight;
    if (WaterPumpManager::waterLevel > 100)
      CalculateWaterLevel();
  }
  bool ShouldWaterPumpStart()
  {
    return WaterPumpManager::waterLevel <= config->minWaterLevel && config->automaticPump;
  }
  void StartPump(byte relayPin)
  {
    waterError = false;
    this->relayPin = relayPin;
    digitalWrite(relayPin, HIGH);
    this->startTimePump = millis();
    this->startWaterLevel = WaterPumpManager::waterLevel;
    isRunning = true;
    this->relayPin = relayPin;
  }
  void CheckWaterLevel()
  {
    if (this->GetPassedSeconds() > config->timeToDetectWaterInsufficiency && WaterPumpManager::waterLevel <= this->startWaterLevel + 10)
    {
      Serial.println("Error in water");
      this->HandleWaterInsufficiency();
    }
    else if (WaterPumpManager::waterLevel >= config->maxWaterLevel)
    {
      StopPump();
    }
    else if (WaterPumpManager::waterLevel > startWaterLevel)
    { // If the waterLevel updated and incremented in level, reset timer
      this->startTimePump = millis();
      startWaterLevel = WaterPumpManager::waterLevel;
    }
  }
  void StopPump()
  {
    isRunning = false;
    digitalWrite(this->relayPin, LOW);
    this->startTimePump = millis();
    this->startWaterLevel = WaterPumpManager::waterLevel;
  }

private:
  void HandleWaterInsufficiency()
  {
    config->automaticPump = false;
    StopPump();
    waterError = true;
  }
  int GetPassedSeconds()
  {
    return (millis() - startTimePump) / 1000;
  }
  // static float GetDistance(byte triggerPin, byte echoPin)
  // {
  //   auto sum = 0;
  //   auto iterations = 50;
  //   for(auto i=0; i < iterations; i++) {
  //     digitalWrite(triggerPin, LOW);
  //     delayMicroseconds(2);
  //     digitalWrite(triggerPin, HIGH);
  //     delayMicroseconds(10);
  //     digitalWrite(triggerPin, LOW);
  //     auto duration = pulseIn(echoPin, HIGH);
  //     sum += duration;
  //   }
  //   auto avgDuration = sum / iterations;
  //   auto distance = (avgDuration / 29.4) / 2;
  //   delay(1000);
  //   return distance;
  // }
  unsigned long GetDistance()
  {
    delay(50);
    auto distance =  this->newPing->convert_cm(newPing->ping_median(10, config->waterTankHeight));
    Serial.println(distance);
    return distance;
  }
};
float WaterPumpManager::waterLevel = 0;
bool WaterPumpManager::isRunning = false;
bool WaterPumpManager::waterError = false;
#endif
