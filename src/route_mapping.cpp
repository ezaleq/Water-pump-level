#include "../includes/server_manager.hpp"


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