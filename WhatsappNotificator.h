#ifndef WHATSAPP_NOTIFICATOR_H
#define WHATSAPP_NOTIFICATOR_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

struct WhatsappConfiguration
{
  char Sid[40];
  char AuthToken[40];
  String From;
  String* To;
};

class WhatsappNotificator {
  public:
    WhatsappNotificator(WhatsappConfiguration configuration) {
      this->configuration = configuration;
    }

    void SendMessage(String message) {
      for (byte i = 0; i < sizeof(this->configuration.To) / sizeof(String); i++) {
        Serial.print("Setuping client for phone");
        Serial.println(this->configuration.To[i]);
        auto httpClient = SetupClient(this->configuration.To[i], message);
        httpClient->POST("");
        httpClient->end();
        delete &httpClient;
      }
    }
  private:
    HTTPClient* SetupClient(String toNumber, String message) {
      String url = "https://api.twilio.com/2010-04-01/Accounts/$Sid/Messages.json?From=$from&To=$to&Body=$body";
      url.replace("$Sid", this->configuration.Sid);
      url.replace("$from", this->configuration.From);
      url.replace("$body", message);
      url.replace("$to", toNumber);
      char* buff = new char[url.length() + 1];
      url.toCharArray(buff, url.length());
      HTTPClient* httpClient = new HTTPClient();
      httpClient->begin(this->wifiClient, buff);
      httpClient->addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpClient->setAuthorization(configuration.Sid, configuration.AuthToken);
      delete buff;
      return httpClient;
    }
    WiFiClient wifiClient;
    WhatsappConfiguration configuration;
};

#endif
