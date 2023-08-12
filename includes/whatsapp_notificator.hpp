#ifndef WHATSAPP_NOTIFICATOR_H
#define WHATSAPP_NOTIFICATOR_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "structs/whatsapp_configuration.hpp"

WiFiClientSecure client;

class WhatsappNotificator
{
public:
  WhatsappNotificator(WhatsappConfiguration* configuration)
  {
    this->config = configuration;
    this->setupClient();
  }

  void sendMessage(String message)
  {
    for (auto i = 0; i < config->toLength; i++)
    {
      setupClient();
      String payload = "From=whatsapp:$from&To=whatsapp:$to&Body=$body";
      payload.replace("$from", config->from);
      payload.replace("$to", config->to[i]);
      payload.replace("$body", message);
      payload.replace("+", "%2B");
      auto httpCode = this->httpClient.POST(payload);
      Serial.println("HTTP Code: " + String(httpCode));
      if (httpCode != HTTP_CODE_CREATED)
      {
        Serial.println("Response" + this->httpClient.getString());
      }
      this->httpClient.end();
    }
  }

private:
  void setupClient()
  {
    client.setInsecure();
    String url = "https://api.twilio.com/2010-04-01/Accounts/$Sid/Messages.json";
    url.replace("$Sid", config->sid);
    this->httpClient.begin(client, url);
    this->httpClient.addHeader("content-type", "application/x-www-form-urlencoded");
    this->httpClient.setAuthorization(config->sid.c_str(), config->authToken.c_str());
  }
  unsigned int arrayLength = 0;
  WhatsappConfiguration *config;
  HTTPClient httpClient;
};

#endif
