#include "src/ServerManager.h"
#include "src/WaterPumpManager.h"
#include "src/WhatsappNotificator.h"
ServerManager serverManager;

auto t0 = millis();
auto t1 = t0;
byte echoPin = D5;
byte triggerPin = D6;
byte relayPin = D7;
bool alreadySentMessage = false;
WaterPumpManager *waterPumpManager;
WhatsappNotificator *wppNotificator;

void setup()
{
  Serial.begin(9600);
  // Initialize the output variables as outputs
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, 0);
  // Start server
  serverManager.Initialize();
  serverManager.ConnectToWiFi();
  serverManager.Start();
  // Create whatsappNotificator and waterPumpManager
  wppNotificator = new WhatsappNotificator(ServerManager::wppConfig.get());
  waterPumpManager = new WaterPumpManager(ServerManager::waterPumpConfig.get(), triggerPin, echoPin);
}

void loop()
{
  waterPumpManager->CalculateWaterLevel();
  if (WaterPumpManager::isRunning && ServerManager::waterPumpConfig->automaticPump)
  {
    waterPumpManager->CheckWaterLevel();
    alreadySentMessage = false;
  }
  else if (waterPumpManager->ShouldWaterPumpStart())
  {
    alreadySentMessage = false;
    waterPumpManager->StartPump(relayPin);
  }
  else if (WaterPumpManager::waterError && !alreadySentMessage)
  {
    wppNotificator->SendMessage("Se detectó una insuficiencia de corriente de agua en la red.\n Los sistemas automáticos de prendido de bomba fueron deshabilitados. Habilitelos nuevamente desde la página web una vez solucionado el problema.");
    alreadySentMessage = true;
  }
  else if (ServerManager::waterPumpConfig->automaticPump == false)
  {
    waterPumpManager->CalculateWaterLevel();
    waterPumpManager->StopPump();
  }
}
