#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "structs/waterpump_configuration.hpp"
#include "structs/whatsapp_configuration.hpp"
#include "waterpump_manager.hpp"

class WaterPumpManager;

class ServerManager
{
public:
  void initialize(uint16_t port = 80);
  void connectToWiFi();
  void start();

  AsyncWebServer *server;
  static std::shared_ptr<WhatsappConfiguration> wppConfig;
  static std::shared_ptr<WaterPumpConfiguration> waterPumpConfig;

private:
  void loadConfiguration();
  void loadTwilioConfiguration();
  void loadWaterPumpConfiguration();
  void loadWifiConfiguration();

private:
  void mapRoutes();
  static void handleGetRealDistance(AsyncWebServerRequest *request);
  static void handleGetCurrentWaterLevel(AsyncWebServerRequest *request);
  static void handleGetConfiguration(AsyncWebServerRequest *request);
  static void handlePostConfiguration(AsyncWebServerRequest *request);

private:
  static void savePumpConfiguration();
  static void saveTwilioConfiguration();
  static void deserializeTwilioConfiguration(AsyncWebServerRequest *request);
  static void deserializePumpConfiguration(AsyncWebServerRequest *request);

  String ssid;
  String password;
  uint16_t port = 80;
};

#endif
