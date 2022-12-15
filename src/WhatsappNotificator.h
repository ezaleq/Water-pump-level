#ifndef WHATSAPP_NOTIFICATOR_H
#define WHATSAPP_NOTIFICATOR_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "structs/WhatsappConfiguration.h"

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
    Serial.println("Sending message  '" + message +"'.");
    for (unsigned int i = 0; i < config->ToLength; i++)
    {
      Serial.println("Iterating i=" + String(i));
      String payload = "From=$from&To=$to&Body=$body";
      config->From.replace("+", "%2B");
      config->To[i].replace("+", "%2B");
      payload.replace("$from", config->From);
      payload.replace("$to", config->To[i]);
      payload.replace("$body", message);
      Serial.println(payload);
      auto httpCode = this->httpClient.POST(payload);
      auto response = this->httpClient.getString();
      Serial.println("HttpCode: " + String(httpCode));
      Serial.println("Response: " + response);
      httpClient.end();
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
