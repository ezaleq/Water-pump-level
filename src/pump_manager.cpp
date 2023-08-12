#include "../includes/waterpump_manager.hpp"

float WaterPumpManager::waterLevel = 0;
bool WaterPumpManager::isRunning = false;
bool WaterPumpManager::waterError = false;
int WaterPumpManager::distanceInCm = 0;

void WaterPumpManager::handleWaterInsufficiency()
{
    config->automaticPump = false;
    stopPump();
    waterError = true;
}

int WaterPumpManager::getPassedSeconds()
{
    return (millis() - startTimePump) / 1000;
}

bool WaterPumpManager::shouldWaterPumpStart()
{
    return WaterPumpManager::waterLevel <= config->minWaterLevel && config->automaticPump;
}
void WaterPumpManager::startPump(byte relayPin)
{
    waterError = false;
    this->relayPin = relayPin;
    digitalWrite(relayPin, HIGH);
    this->startTimePump = millis();
    this->startWaterLevel = WaterPumpManager::waterLevel;
    isRunning = true;
    this->relayPin = relayPin;
}
void WaterPumpManager::checkWaterLevel()
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
void WaterPumpManager::stopPump()
{
    isRunning = false;
    digitalWrite(this->relayPin, LOW);
    this->startTimePump = millis();
    this->startWaterLevel = WaterPumpManager::waterLevel;
}