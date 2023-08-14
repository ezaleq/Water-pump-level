#include "../includes/server_manager.hpp"

void ServerManager::initialize(uint16_t port)
{
    this->port = port;
    this->loadConfiguration();
    this->mapRoutes();
}

void ServerManager::connectToWiFi()
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

void ServerManager::start()
{
    this->server->begin();
    Serial.print("Server started on IP ");
    Serial.print(WiFi.localIP());
    Serial.print(" on port ");
    Serial.println(this->port);
}