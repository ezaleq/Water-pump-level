#include "includes/server_manager.hpp"
#include "includes/whatsapp_notificator.hpp"
#include "includes/waterpump_manager.hpp"
#include <AsyncElegantOTA.h>

ServerManager serverManager;

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
  serverManager.initialize();
  serverManager.connectToWiFi();
  serverManager.start();
  // Create whatsappNotificator and waterPumpManager
  waterPumpManager = new WaterPumpManager(ServerManager::waterPumpConfig.get(), triggerPin, echoPin);
  wppNotificator = new WhatsappNotificator(ServerManager::wppConfig.get());
  AsyncElegantOTA.begin(serverManager.server);
}

void loop()
{
  AsyncElegantOTA.loop();
  waterPumpManager->calculateWaterLevel();
  if (WaterPumpManager::isRunning && ServerManager::waterPumpConfig->automaticPump)
  {
    waterPumpManager->checkWaterLevel();
    alreadySentMessage = false;
  }
  else if (waterPumpManager->shouldWaterPumpStart())
  {
    alreadySentMessage = false;
    waterPumpManager->startPump(relayPin);
  }
  else if (WaterPumpManager::waterError && !alreadySentMessage)
  {
    wppNotificator->sendMessage("Se detectó una insuficiencia de corriente de agua en la red.\n Los sistemas automáticos de prendido de bomba fueron deshabilitados. Habilitelos nuevamente desde la página web una vez solucionado el problema.");
    alreadySentMessage = true;
  }
  else if (ServerManager::waterPumpConfig->automaticPump == false)
  {
    waterPumpManager->stopPump();
  }
}
