#include "WaterPumpManager.h"
#include "ServerManager.h"

ServerManager serverManager;

void setup() {
  Serial.begin(9600);
  // Initialize the output variables as outputs
  serverManager.ConnectToWiFi("Quevedo's Family", "42393445");
  serverManager.Initialize();
  serverManager.Start(); 
}

void loop(){
  WaterPumpManager::waterLevel = WaterPumpManager::waterLevel + 1;
}
