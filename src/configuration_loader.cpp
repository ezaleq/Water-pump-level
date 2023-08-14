#include "../includes/server_manager.hpp"
#include "../includes/file_manager.hpp"

void ServerManager::loadConfiguration()
{
    FileManager::init();
    loadWifiConfiguration();
    loadTwilioConfiguration();
    loadWaterPumpConfiguration();
}

void ServerManager::loadTwilioConfiguration()
{
    FileManager::parseFile(
        "/twilio.dat",
        1024,
        512,
        [this](DynamicJsonDocument &doc)
        {
            this->wppConfig->sid = (String)doc["sid"];
            this->wppConfig->authToken = (String)doc["authToken"];
            this->wppConfig->from = (String)doc["from"];
            this->wppConfig->toLength = (int)doc["toLength"];
            size_t i = 0;
            JsonArray array = doc["to"];
            this->wppConfig->to = new String[this->wppConfig->toLength];
            for (auto i = 0; i < this->wppConfig->toLength; i++)
            {
                String to = array[i];
                this->wppConfig->to[i] = to;
            }
        });
    if (this->wppConfig == nullptr)
    {
        Serial.println("Error parsing twilio.dat");
    }
}

void ServerManager::loadWaterPumpConfiguration()
{
    FileManager::parseFile(
        "/pump.dat",
        1024,
        96,
        [this](DynamicJsonDocument &doc)
        {
            waterPumpConfig->automaticPump = (bool)doc["automaticPump"];
            waterPumpConfig->minWaterLevel = (byte)doc["minWaterLevel"];
            waterPumpConfig->maxWaterLevel = (byte)doc["maxWaterLevel"];
            waterPumpConfig->timeToDetectWaterInsufficiency = (unsigned short int)doc["timeToDetectWaterInsufficiency"];
            waterPumpConfig->waterTankHeight = (unsigned short int)doc["waterTankHeight"];
        });
}

void ServerManager::loadWifiConfiguration()
{
    FileManager::parseFile(
        "/wifi.dat",
        1024,
        32,
        [this](DynamicJsonDocument &doc)
        {
            this->ssid = (String)doc["ssid"];
            this->password = (String)doc["password"];
        });
    if (this->ssid == nullptr || this->password == nullptr)
    {
        Serial.println("Error parsing wifi.dat");
        ESP.reset();
    }
}