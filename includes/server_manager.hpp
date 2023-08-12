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
  void connectToWiFi()
  {
    Serial.print("\nConnecting to ");
    Serial.println(this->ssid);
    WiFi.begin(this->ssid.c_str(), this->password.c_str());
    Serial.println("Waiting to connect .");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
  }
  void initialize(uint16_t port = 80)
  {
    this->port = port;
    this->initializeSpiffs();
    this->loadConfiguration();
    this->mapRoutes();
  }

  void loadConfiguration()
  {
    loadWifiConfiguration();
    loadTwilioConfiguration();
    loadWaterPumpConfiguration();
  }

  void loadTwilioConfiguration()
  {
    Serial.println("==========================");
    Serial.println("Loading twilio config");
    File file = SPIFFS.open("/twilio.dat", "r");
    if (!file)
    {
      Serial.println("Error opening twilio.dat");
      return;
    }
    size_t size = file.size();
    if (size > 1024)
    {
      Serial.println("twilio.dat file size is too large");
      return;
    }
    std::shared_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error)
    {
      Serial.println("Error parsing twilio.dat");
      return;
    }
    String accountSid = doc["sid"];
    String authToken = doc["authToken"];
    String fromNumber = doc["from"];
    int toLength = doc["toLength"];
    wppConfig->sid = accountSid;
    wppConfig->authToken = authToken;
    wppConfig->from = fromNumber;
    wppConfig->toLength = toLength;
    size_t i = 0;
    JsonArray array = doc["to"];
    wppConfig->to = new String[toLength];

    for (auto i = 0; i < toLength; i++)
    {
      String to = array[i];
      Serial.println(to);
      wppConfig->to[i] = to;
    }
    Serial.println("-> AccountSid: " + wppConfig->sid);
    Serial.println("-> AuthToken: " + wppConfig->authToken);
    Serial.println("-> From: " + wppConfig->from);
    Serial.println("-> ToLength: " + String(wppConfig->toLength));
    for (auto i = 0; i < wppConfig->toLength; i++)
    {
      Serial.println("-> To[" + String(i) + "]: " + wppConfig->to[i]);
    }
    Serial.println("==========================");
  }

  void loadWaterPumpConfiguration()
  {
    Serial.println("----------------------------------------");
    Serial.println("Loading pump config... ");
    File file = SPIFFS.open("/pump.dat", "r");
    if (!file)
    {
      Serial.println("Error opening pump.dat");
      return;
    }
    size_t size = file.size();
    if (size > 1024)
    {
      Serial.println("pump.dat file size is too large");
      return;
    }
    std::shared_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    DynamicJsonDocument doc(96);
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error)
    {
      Serial.println("Error parsing pump.dat");
      return;
    }
    bool automaticPump = doc["automaticPump"];
    byte minWaterLevel = doc["minWaterLevel"];
    byte maxWaterLevel = doc["maxWaterLevel"];
    unsigned short int timeToDetectWaterInsufficiency = doc["timeToDetectWaterInsufficiency"];
    unsigned short int waterTankHeight = doc["waterTankHeight"];
    waterPumpConfig->automaticPump = automaticPump;
    waterPumpConfig->minWaterLevel = minWaterLevel;
    waterPumpConfig->maxWaterLevel = maxWaterLevel;
    waterPumpConfig->timeToDetectWaterInsufficiency = timeToDetectWaterInsufficiency;
    waterPumpConfig->waterTankHeight = waterTankHeight;
    Serial.println("-> AutomaticPump: " + String(waterPumpConfig->automaticPump));
    Serial.println("-> MinWaterLevel: " + String(waterPumpConfig->minWaterLevel));
    Serial.println("-> MaxWaterLevel: " + String(waterPumpConfig->maxWaterLevel));
    Serial.println("-> TimeToDetectWaterInsufficiency: " + String(waterPumpConfig->timeToDetectWaterInsufficiency));
    Serial.println("-> WaterTankHeight: " + String(waterPumpConfig->waterTankHeight));
    Serial.println("----------------------------------------");
  }

  void loadWifiConfiguration()
  {
    Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$");
    Serial.println("Loading WiFi config...");
    File file = SPIFFS.open("/wifi.dat", "r");
    if (!file)
    {
      Serial.println("Error opening wifi.dat");
      return;
    }
    size_t size = file.size();
    if (size > 1024)
    {
      Serial.println("wifi.dat file size is too large");
      return;
    }
    std::shared_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    DynamicJsonDocument json(32);
    DeserializationError error = deserializeJson(json, buf.get());
    if (error)
    {
      Serial.println("Error parsing wifi.dat");
      return;
    }
    JsonObject obj = json.as<JsonObject>();
    String ssid = obj[String("ssid")];
    String password = obj[String("password")];
    this->ssid = ssid;
    this->password = password;
    Serial.println("$$$$$$$$$$$$$$$$$$$$$$$$$");
  }

  void start()
  {
    this->server->begin();
    Serial.print("Server started on IP ");
    Serial.print(WiFi.localIP());
    Serial.print(" on port ");
    Serial.println(this->port);
  }
  AsyncWebServer *server;
  static std::shared_ptr<WhatsappConfiguration> wppConfig;
  static std::shared_ptr<WaterPumpConfiguration> waterPumpConfig;

private:
  void initializeSpiffs()
  {
    if (!SPIFFS.begin())
    {
      Serial.println("Error montando el administrador de archivos SPIFFS");
      return;
    }
  }
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
