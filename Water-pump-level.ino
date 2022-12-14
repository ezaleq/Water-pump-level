#include "WaterPumpManager.h"
#include "ServerManager.h"

ServerManager serverManager;

unsigned long startTime = millis();
byte echoPin = D5;
byte triggerPin = D6;


void setup() {
  Serial.begin(9600);
  // Initialize the output variables as outputs
  serverManager.ConnectToWiFi("Quevedo's Family", "42393445");
  serverManager.Initialize();
  serverManager.Start(); 
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
}

void loop(){
  WaterPumpManager::calculateWaterLevel(triggerPin, echoPin);
}
