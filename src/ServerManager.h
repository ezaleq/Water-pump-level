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
    Serial.println("Loading twilio config...");
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
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error)
    {
      Serial.println("Error parsing twilio.dat");
      return;
    }
    String accountSid = doc["sid"];
    String authToken = doc["authToken"];
    String fromNumber = doc["from"];
    wppConfig->Sid = accountSid;
    wppConfig->AuthToken = authToken;
    wppConfig->From = fromNumber;
    size_t i = 0;
    auto array = doc["to"].as<JsonArray>();
    auto arraySize = array.size();
    wppConfig->ToLength = arraySize;
    wppConfig->To = new String[arraySize];

    for(auto phoneNumber : array)
    {
      String to = phoneNumber.as<String>();
      Serial.println(to);
      i++;
    }
    Serial.println("-> AccountSid: " + wppConfig->Sid);
    Serial.println("-> AuthToken: " + wppConfig->AuthToken);
    Serial.println("-> From: " + wppConfig->From);
    Serial.println("-> To[0]: " + wppConfig->To[0]);
  }

  void LoadWaterPumpConfiguration()
  {
    Serial.println("Loading pump config...");
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
  }

  void LoadWifiConfiguration()
  {
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
    StaticJsonDocument<96> json;
    json["automaticPump"] = waterPumpConfig->automaticPump;
    json["minWaterLevel"] = waterPumpConfig->minWaterLevel;
    json["maxWaterLevel"] = waterPumpConfig->maxWaterLevel;
    json["timeToDetectWaterInsufficiency"] = waterPumpConfig->timeToDetectWaterInsufficiency;
    json["waterTankHeight"] = waterPumpConfig->waterTankHeight;
  }
  String ssid;
  String password;
  uint16_t port = 80;
};

std::shared_ptr<WhatsappConfiguration> ServerManager::wppConfig = std::make_shared<WhatsappConfiguration>();
std::shared_ptr<WaterPumpConfiguration> ServerManager::waterPumpConfig = std::make_shared<WaterPumpConfiguration>();

#endif

