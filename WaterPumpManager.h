#ifndef WATER_PUMP_MANAGER_H
#define WATER_PUMP_MANAGER_H

#include "WhatsappNotificator.h"

class WaterPumpManager {
  public:
    static bool isRunning;
    static float waterLevel;
    static bool automaticPump;
    static byte minWaterLevel;
    static byte maxWaterLevel;
    static unsigned short int timeToDetectWaterInsufficiency; // in seconds
    static unsigned short int waterTankHeight; // in cm

    WaterPumpManager(WhatsappConfiguration wppConfig, unsigned int arrayLength) {
      this->wppNotificator = new WhatsappNotificator(wppConfig, arrayLength);
    }
    static void CalculateWaterLevel(byte triggerPin, byte echoPin) {
      auto distance = GetDistance(triggerPin, echoPin);
      waterLevel = 100 - (distance * 100) / waterTankHeight;
      if (waterLevel > 100)
        CalculateWaterLevel(triggerPin, echoPin);
    }
    static bool ShouldWaterPumpStart() {
      return waterLevel <= minWaterLevel && automaticPump;
    }
    void StartPump(byte relayPin) {
      this->relayPin = relayPin;
      digitalWrite(relayPin, HIGH);
      this->startTimePump = millis();
      this->startWaterLevel = waterLevel;
      isRunning = true;
      this->relayPin = relayPin;
    }
    void CheckWaterLevel() {
      if (this->GetPassedSeconds() > timeToDetectWaterInsufficiency && waterLevel <= this->startWaterLevel + 10 ) {
        Serial.println("Error in water");
        this->HandleWaterInsufficiency();
      }
      else if (waterLevel >= maxWaterLevel) {
        StopPump();
      }
      else if (waterLevel > startWaterLevel) { // If the waterLevel updated and incremented in level, reset timer
        this->startTimePump = millis();
        startWaterLevel = waterLevel;
      }
    }
    void StopPump() {
      isRunning = false;
      digitalWrite(this->relayPin, LOW);
      this->startTimePump = millis();
      this->startWaterLevel = waterLevel;
    }
  private:
    unsigned long startTimePump = millis();
    float startWaterLevel = 0;
    byte relayPin = 0;
    WhatsappNotificator *wppNotificator;
    void HandleWaterInsufficiency() {
      automaticPump = false;
      StopPump();
      Serial.println("Sending message...");
      wppNotificator->SendMessage("Se detectó una insuficiencia de corriente de agua en la red.\n Los sistemas automáticos de prendido de bomba fueron deshabilitados. Habilitelos nuevamente desde la página web una vez solucionado el problema.");
    }
    int GetPassedSeconds() {
      return (millis() - startTimePump) / 1000;
    }
    static float GetDistance(byte triggerPin, byte echoPin) {
      digitalWrite(triggerPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(triggerPin, LOW);
      auto duration = pulseIn(echoPin, HIGH);
      auto distance = ( duration / 29 ) / 2;
      delay(1000);
      return distance;
    }
};
bool WaterPumpManager::isRunning = false;
byte WaterPumpManager::minWaterLevel = 20;
byte WaterPumpManager::maxWaterLevel = 60;
bool WaterPumpManager::automaticPump = true;
float WaterPumpManager::waterLevel = 0;
unsigned short int WaterPumpManager::timeToDetectWaterInsufficiency = 30; // In seconds
unsigned short int WaterPumpManager::waterTankHeight = 9;
#endif
