#ifndef WATER_PUMP_CONFIGURATION_H
#define WATER_PUMP_CONFIGURATION_H

struct WaterPumpConfiguration
{
  bool automaticPump = true;
  byte minWaterLevel = 30;
  byte maxWaterLevel = 80;
  unsigned short int timeToDetectWaterInsufficiency = 60; // in seconds
  unsigned short int waterTankHeight = 9;                 // in cm
};

#endif