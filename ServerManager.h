#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

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
  void CreateServer(uint16_t port = 80) {
    this->StartSpiffs();
    this->StartServer(port);
    this->CloseSpiffs();
   
  }
private:
  void StartSpiffs() {
    if (!SPIFFS.begin()) {
      Serial.println("Error montando el administrador de archivos SPIFFS");
      return;
    }
  }
  void StartServer(uint16_t port) {
    Serial.println("Starting server...");
    this->server = new AsyncWebServer(port);
    this->server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", String(), false);
    });
    this->server->begin();
    Serial.print("Server started on IP ");
    Serial.print(WiFi.localIP());
    Serial.print(" on port ");
    Serial.println(port);
  }
  void CloseSpiffs() {
     
  }
  char* ssid;
  char* password;
  AsyncWebServer* server;
  
  
};
