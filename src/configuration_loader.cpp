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
    FileManager::parseFile("/twilio.dat", 1024, 512, [this](DynamicJsonDocument &doc)
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
      } });
    if (this->wppConfig == nullptr)
    {
        Serial.println("Error parsing twilio.dat");
    }
}

void ServerManager::loadWaterPumpConfiguration()
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

void ServerManager::loadWifiConfiguration()
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