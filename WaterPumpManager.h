#ifndef WATER_PUMP_MANAGER_H
#define WATER_PUMP_MANAGER_H

class WaterPumpManager {
  public:
    static float waterLevel;
    static bool automaticPump;
    static byte minWaterLevel;
    static byte maxWaterLevel;
    static unsigned short int timeToDetectWaterInsufficiency; // in seconds
    static unsigned short int waterTankHeight; // in cm
    static void calculateWaterLevel(byte triggerPin, byte echoPin) {
      auto distance = getDistance(triggerPin, echoPin);
      waterLevel = 100 - (distance * 100) / waterTankHeight; 
    }
  private:
    static float getDistance(byte triggerPin, byte echoPin) {
      digitalWrite(triggerPin, LOW);
      delayMicroseconds(2);
      digitalWrite(triggerPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(triggerPin, LOW);
      auto duration = pulseIn(echoPin, HIGH);
      auto distance = ( duration / 29 ) / 2;
      return distance;
    }
};

byte WaterPumpManager::minWaterLevel = 50;
byte WaterPumpManager::maxWaterLevel = 80;
bool WaterPumpManager::automaticPump = true;
float WaterPumpManager::waterLevel = 0;
unsigned short int WaterPumpManager::timeToDetectWaterInsufficiency = 30; // In seconds
unsigned short int WaterPumpManager::waterTankHeight = 9;
#endif
