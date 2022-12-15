#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include "structs/WaterPumpConfiguration.h"
#include "structs/WhatsappConfiguration.h"
#include "WaterPumpManager.h"

class WaterPumpManager;

class ServerManager
{
public:
  void ConnectToWiFi()
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
  void Initialize(uint16_t port = 80)
  {
    this->port = port;
    this->InitializeSpiffs();
    this->LoadConfiguration();
    this->InitializeServer();
  }

  void LoadConfiguration()
  {
    LoadWifiConfiguration();
    LoadTwilioConfiguration();
    LoadWaterPumpConfiguration();
  }

  void LoadTwilioConfiguration()
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
    wppConfig->Sid = accountSid;
    wppConfig->AuthToken = authToken;
    wppConfig->From = fromNumber;
    wppConfig->ToLength = toLength;
    size_t i = 0;
    JsonArray array = doc["to"];
    wppConfig->To = new String[toLength];

    for (auto i = 0; i < toLength; i++)
    {
      String to = array[i];
      Serial.println(to);
      wppConfig->To[i] = to;
    }
    Serial.println("-> AccountSid: " + wppConfig->Sid);
    Serial.println("-> AuthToken: " + wppConfig->AuthToken);
    Serial.println("-> From: " + wppConfig->From);
    Serial.println("-> ToLength: " + String(wppConfig->ToLength));
    for (auto i = 0; i < wppConfig->ToLength; i++)
    {
      Serial.println("-> To[" + String(i) + "]: " + wppConfig->To[i]);
    }
    Serial.println("==========================");
  }

  void LoadWaterPumpConfiguration()
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

  void LoadWifiConfiguration()
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

  void Start()
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
  void InitializeSpiffs()
  {
    if (!SPIFFS.begin())
    {
      Serial.println("Error montando el administrador de archivos SPIFFS");
      return;
    }
  }
  void InitializeServer()
  {
    Serial.println("Initializing server...");
    this->server = new AsyncWebServer(this->port);
    this->server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                     { request->send(SPIFFS, "/index.html", String(), false); });
    this->server->on("/api/configuration", HTTP_GET, HandleGetConfiguration);
    this->server->on("/api/configuration", HTTP_POST, HandlePostConfiguration);
    this->server->on("/api/current-water-level", HTTP_GET, HandleGetCurrentWaterLevel);
  }

  static void HandleGetCurrentWaterLevel(AsyncWebServerRequest *request)
  {
    String response;
    StaticJsonDocument<48> json;
    json["waterLevel"] = WaterPumpManager::waterLevel;
    json["pumpRunning"] = WaterPumpManager::isRunning;
    json["waterError"] = WaterPumpManager::waterError;
    serializeJson(json, response);
    request->send(200, "application/json", response);
  }

  static void HandleGetConfiguration(AsyncWebServerRequest *request)
  {
    String response;
    StaticJsonDocument<384> json;
    auto object = json.to<JsonObject>();
    object["automaticPump"] = waterPumpConfig->automaticPump;
    object["pumpMinTrigger"] = waterPumpConfig->minWaterLevel;
    object["pumpMaxTrigger"] = waterPumpConfig->maxWaterLevel;
    object["pumpStopTrigger"] = waterPumpConfig->timeToDetectWaterInsufficiency;
    object["tankHeight"] = waterPumpConfig->waterTankHeight;
    object["Sid"] = wppConfig->Sid;
    object["token"] = wppConfig->AuthToken;
    object["fromPhone"] = wppConfig->From;
    auto arr = object.createNestedArray("toPhone");
    for (int i = 0; i < wppConfig->ToLength; i++)
    {
      arr.add(wppConfig->To[i]);
    }

    serializeJson(json, response);
    request->send(200, "application/json", response);
  }

  static void SavePumpConfiguration()
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

  static void SaveTwilioConfiguration()
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
    doc["sid"] = wppConfig->Sid;
    doc["authToken"] = wppConfig->AuthToken;
    doc["from"] = wppConfig->From;
    doc["toLength"] = wppConfig->ToLength;
    Serial.println("Adding phone numbers to JsonArray...");
    for (int i = 0; i < wppConfig->ToLength; i++)
    {
      Serial.println("Adding " + wppConfig->To[i]);
      arr.add(wppConfig->To[i]);
    }
    serializeJson(doc, file);
    file.close();
  }

  static void DeserializeTwilioConfiguration(AsyncWebServerRequest *request)
  {
    Serial.println("Obtaining twilio configuration from post request...");
    auto sid = request->getParam("sid", true)->value();
    auto authToken = request->getParam("authToken", true)->value();
    auto from = request->getParam("from", true)->value();
    auto to = request->getParam("to", true)->value();
    auto commasCount = std::count(to.begin(), to.end(), ',');
    wppConfig->ToLength = 0;
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
      wppConfig->ToLength = commasCount + 1;
    }
    else if (to.length() > 0)
    {
      commasCount = 1;
      toArr = new String[1];
      toArr[0] = to;
      wppConfig->ToLength = 1;
    }
    Serial.println("- Sid: " + sid);
    Serial.println("- AuthToken: " + authToken);
    Serial.println("- From: " + from);
    for (auto i = 0; i < wppConfig->ToLength; i++)
    {
      Serial.println("- To[" + String(i) + "] = " + toArr[i]);
    }
    int count = 0;

    wppConfig->Sid = sid;
    wppConfig->AuthToken = authToken;
    wppConfig->From = from;
    wppConfig->To = toArr;
  }

  static void DeserializePumpConfiguration(AsyncWebServerRequest *request)
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

  static void HandlePostConfiguration(AsyncWebServerRequest *request)
  {
    if (request->hasParam("automaticPump", true) && request->hasParam("minWaterLevel", true) && request->hasParam("maxWaterLevel", true) && request->hasParam("timeToDetectWaterInsufficiency", true) && request->hasParam("waterTankHeight", true) && request->hasParam("sid", true) && request->hasParam("authToken", true) && request->hasParam("from", true) && request->hasParam("to", true))
    {
      DeserializeTwilioConfiguration(request);
      DeserializePumpConfiguration(request);
      SaveTwilioConfiguration();
      SavePumpConfiguration();
      request->send(200, "application/json", "{\"status\": \"ok\"}");
    }
    else
    {
      request->send(400, "application/json", "{\"status\": \"error\"}");
    }
  }
  String ssid;
  String password;
  uint16_t port = 80;
};

std::shared_ptr<WhatsappConfiguration> ServerManager::wppConfig = std::make_shared<WhatsappConfiguration>();
std::shared_ptr<WaterPumpConfiguration> ServerManager::waterPumpConfig = std::make_shared<WaterPumpConfiguration>();

#endif
