#include "WhatsappNotificator.h"
#include "WaterPumpManager.h"
#include "ServerManager.h"
ServerManager serverManager;

auto t0 = millis();
auto t1 = t0;
byte echoPin = D5;
byte triggerPin = D6;
byte relayPin = D7;
WaterPumpManager* waterPumpManager;
String phones[] = {"whatsapp:+5491154541063"};
WhatsappConfiguration wppConfig = {
  "AC46f7092d6da6bc5f1f714e8a9573d90b",
  "46f9d55b4d1247a36aa66c09ac714c05",
  "whatsapp:+14155238886",
  phones
};

void setup() {
  Serial.begin(9600);
  waterPumpManager = new WaterPumpManager(wppConfig, 1);
  // Initialize the output variables as outputs
  serverManager.ConnectToWiFi("Quevedo's Family", "42393445");
  serverManager.Initialize();
  serverManager.Start();
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, 0);
}

void loop() {
  WaterPumpManager::CalculateWaterLevel(triggerPin, echoPin);
  if (WaterPumpManager::isRunning && WaterPumpManager::automaticPump) {
    waterPumpManager->CheckWaterLevel();
  }
  else if (WaterPumpManager::ShouldWaterPumpStart()) {
    waterPumpManager->StartPump(relayPin);
  }
}
