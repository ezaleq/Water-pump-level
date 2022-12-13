#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include "WaterPumpManager.h"

class ServerManager {
  public:
    void ConnectToWiFi(String ssid, String password) {
      Serial.println("\nConnecting to " + ssid);
      WiFi.begin(ssid, password);
      Serial.println("Waiting to connect .");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    }
    void Initialize(uint16_t port = 80) {
      this->port = port;
      this->InitializeSpiffs();
      this->InitializeServer();
    }

    void Start() {
      this->server->begin();
      Serial.print("Server started on IP ");
      Serial.print(WiFi.localIP());
      Serial.print(" on port ");
      Serial.println(this->port);
    }
    AsyncWebServer* server;
  private:
    void InitializeSpiffs() {
      if (!SPIFFS.begin()) {
        Serial.println("Error montando el administrador de archivos SPIFFS");
        return;
      }
    }
    void InitializeServer() {
      Serial.println("Initializing server...");
      this->server = new AsyncWebServer(this->port);
      this->server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(SPIFFS, "/index.html", String(), false);
      });
      this->server->on("/api/current-water-level", HTTP_GET, HandleGetCurrentWaterLevel);
    }

    static void HandleGetCurrentWaterLevel(AsyncWebServerRequest *request) {
      String response;
      StaticJsonDocument<16> json;
      json["value"] = WaterPumpManager::waterLevel;
      serializeJson(json, response);
      request->send(200, "application/json", response);
    }

    char* ssid;
    char* password;
    uint16_t port = 80;
};

#endif
