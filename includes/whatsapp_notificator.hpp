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
    this->SetupClient();
  }

  void SendMessage(String message)
  {
    for (auto i = 0; i < config->ToLength; i++)
    {
      SetupClient();
      String payload = "From=whatsapp:$from&To=whatsapp:$to&Body=$body";
      payload.replace("$from", config->From);
      payload.replace("$to", config->To[i]);
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
  void SetupClient()
  {
    client.setInsecure();
    String url = "https://api.twilio.com/2010-04-01/Accounts/$Sid/Messages.json";
    url.replace("$Sid", config->Sid);
    this->httpClient.begin(client, url);
    this->httpClient.addHeader("content-type", "application/x-www-form-urlencoded");
    this->httpClient.setAuthorization(config->Sid.c_str(), config->AuthToken.c_str());
  }
  unsigned int arrayLength = 0;
  WhatsappConfiguration *config;
  HTTPClient httpClient;
};

#endif
