#include "../includes/server_manager.hpp"
#include <ArduinoJson.h>

std::shared_ptr<WhatsappConfiguration> ServerManager::wppConfig = std::make_shared<WhatsappConfiguration>();
std::shared_ptr<WaterPumpConfiguration> ServerManager::waterPumpConfig = std::make_shared<WaterPumpConfiguration>();

void ServerManager::mapRoutes()
{
    Serial.println("Initializing server...");
    this->server = new AsyncWebServer(this->port);
    this->server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                     { request->send(SPIFFS, "/index.html", String(), false); });
    this->server->on("/realdata", HTTP_GET, [](AsyncWebServerRequest *request)
                     { request->send(SPIFFS, "/realdata.html", String(), false); });
    this->server->on("/api/configuration", HTTP_GET, handleGetConfiguration);
    this->server->on("/api/configuration", HTTP_POST, handlePostConfiguration);
    this->server->on("/api/current-water-level", HTTP_GET, handleGetCurrentWaterLevel);
    this->server->on("/api/real-distance", HTTP_GET, handleGetRealDistance);
}

void ServerManager::handleGetRealDistance(AsyncWebServerRequest *request)
{
    auto distance = WaterPumpManager::distanceInCm;
    String distanceString = String(distance);
    request->send(200, "application/json", "{\"distance\": " + distanceString + "}");
}

void ServerManager::handleGetCurrentWaterLevel(AsyncWebServerRequest *request)
{
    String response;
    StaticJsonDocument<48> json;
    json["waterLevel"] = WaterPumpManager::waterLevel;
    json["pumpRunning"] = WaterPumpManager::isRunning;
    json["waterError"] = WaterPumpManager::waterError;
    serializeJson(json, response);
    request->send(200, "application/json", response);
}

void ServerManager::handleGetConfiguration(AsyncWebServerRequest *request)
{
    String response;
    StaticJsonDocument<384> json;
    auto object = json.to<JsonObject>();
    object["automaticPump"] = waterPumpConfig->automaticPump;
    object["pumpMinTrigger"] = waterPumpConfig->minWaterLevel;
    object["pumpMaxTrigger"] = waterPumpConfig->maxWaterLevel;
    object["pumpStopTrigger"] = waterPumpConfig->timeToDetectWaterInsufficiency;
    object["tankHeight"] = waterPumpConfig->waterTankHeight;
    object["Sid"] = wppConfig->sid;
    object["token"] = wppConfig->authToken;
    object["fromPhone"] = wppConfig->from;
    auto arr = object.createNestedArray("toPhone");
    for (int i = 0; i < wppConfig->toLength; i++)
    {
        arr.add(wppConfig->to[i]);
    }

    serializeJson(json, response);
    request->send(200, "application/json", response);
}

void ServerManager::handlePostConfiguration(AsyncWebServerRequest *request)
{
    if (request->hasParam("automaticPump", true) && request->hasParam("minWaterLevel", true) && request->hasParam("maxWaterLevel", true) && request->hasParam("timeToDetectWaterInsufficiency", true) && request->hasParam("waterTankHeight", true) && request->hasParam("sid", true) && request->hasParam("authToken", true) && request->hasParam("from", true) && request->hasParam("to", true))
    {
        deserializeTwilioConfiguration(request);
        deserializePumpConfiguration(request);
        saveTwilioConfiguration();
        savePumpConfiguration();
        request->send(200, "application/json", "{\"status\": \"ok\"}");
    }
    else
    {
        request->send(400, "application/json", "{\"status\": \"error\"}");
    }
}

void ServerManager::deserializeTwilioConfiguration(AsyncWebServerRequest *request)
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

void ServerManager::deserializePumpConfiguration(AsyncWebServerRequest *request)
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
