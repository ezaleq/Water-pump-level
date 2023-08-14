#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
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

  static void savePumpConfiguration()
  {
    Serial.println("Saving pump config...");
    File file = SPIFFS.open("/pump.dat", "w");
    if (!file)
    {
      Serial.println("Error opening pump.dat");
      return;
    }
    StaticJsonDocument<96> json;
    json["automaticPump"] = waterPumpConfig->automaticPump;
    json["minWaterLevel"] = waterPumpConfig->minWaterLevel;
    json["maxWaterLevel"] = waterPumpConfig->maxWaterLevel;
    json["timeToDetectWaterInsufficiency"] = waterPumpConfig->timeToDetectWaterInsufficiency;
    json["waterTankHeight"] = waterPumpConfig->waterTankHeight;
    serializeJson(json, file);
    file.close();
  }

  static void saveTwilioConfiguration()
  {
    Serial.println("Saving Twilio config...");
    File file = SPIFFS.open("/twilio.dat", "w");
    if (!file)
    {
      Serial.println("Error opening twilio.dat");
      return;
    }
    StaticJsonDocument<512> doc;
    auto arr = doc.createNestedArray("to");
    doc["sid"] = wppConfig->sid;
    doc["authToken"] = wppConfig->authToken;
    doc["from"] = wppConfig->from;
    doc["toLength"] = wppConfig->toLength;
    Serial.println("Adding phone numbers to JsonArray...");
    for (int i = 0; i < wppConfig->toLength; i++)
    {
      Serial.println("Adding " + wppConfig->to[i]);
      arr.add(wppConfig->to[i]);
    }
    serializeJson(doc, file);
    file.close();
  }

  static void deserializeTwilioConfiguration(AsyncWebServerRequest *request)
  {
    Serial.println("Obtaining twilio configuration from post request...");
    auto sid = request->getParam("sid", true)->value();
    auto authToken = request->getParam("authToken", true)->value();
    auto from = request->getParam("from", true)->value();
    auto to = request->getParam("to", true)->value();
    auto commasCount = std::count(to.begin(), to.end(), ',');
    wppConfig->toLength = 0;
    String *toArr = NULL;
    if (commasCount > 0)
    {
      toArr = new String[commasCount + 1];
      auto startIndex = 0;
      auto nextIndex = 0;

      for (auto i = 0; i < commasCount; i++)
      {
        nextIndex = to.indexOf(",", startIndex);
        auto substring = to.substring(startIndex, nextIndex);
        toArr[i] = substring;
        startIndex = nextIndex + 1;
      }
      auto lastString = to.substring(startIndex, to.length());
      toArr[commasCount] = lastString;
      wppConfig->toLength = commasCount + 1;
    }
    else if (to.length() > 0)
    {
      commasCount = 1;
      toArr = new String[1];
      toArr[0] = to;
      wppConfig->toLength = 1;
    }
    Serial.println("- Sid: " + sid);
    Serial.println("- AuthToken: " + authToken);
    Serial.println("- From: " + from);
    for (auto i = 0; i < wppConfig->toLength; i++)
    {
      Serial.println("- To[" + String(i) + "] = " + toArr[i]);
    }
    int count = 0;

    wppConfig->sid = sid;
    wppConfig->authToken = authToken;
    wppConfig->from = from;
    wppConfig->to = toArr;
  }

  static void deserializePumpConfiguration(AsyncWebServerRequest *request)
  {
    Serial.println("Obtaining pump configuration from POST request...");
    bool automaticPump = request->getParam("automaticPump", true)->value().toInt() == 1 ? true : false;
    byte minWaterLevel = request->getParam("minWaterLevel", true)->value().toInt();
    byte maxWaterLevel = request->getParam("maxWaterLevel", true)->value().toInt();
    unsigned short int timeToDetectWaterInsufficiency = request->getParam("timeToDetectWaterInsufficiency", true)->value().toInt();
    unsigned short int waterTankHeight = request->getParam("waterTankHeight", true)->value().toInt();
    Serial.println("- AutomaticPump: " + String(automaticPump));
    Serial.println("- MinWaterLevel: " + String(minWaterLevel));
    Serial.println("- MaxWaterLevel: " + String(maxWaterLevel));
    Serial.println("- TimeToDetectWaterInsufficiency: " + String(timeToDetectWaterInsufficiency));
    Serial.println("- WaterTankHeight: " + String(waterTankHeight));

    waterPumpConfig->automaticPump = automaticPump;
    waterPumpConfig->minWaterLevel = minWaterLevel;
    waterPumpConfig->maxWaterLevel = maxWaterLevel;
    waterPumpConfig->timeToDetectWaterInsufficiency = timeToDetectWaterInsufficiency;
    waterPumpConfig->waterTankHeight = waterTankHeight;
  }

  String ssid;
  String password;
  uint16_t port = 80;
};

#endif
