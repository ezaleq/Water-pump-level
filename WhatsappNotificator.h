#ifndef WHATSAPP_NOTIFICATOR_H
#define WHATSAPP_NOTIFICATOR_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


WiFiClientSecure client;

struct WhatsappConfiguration
{
  String Sid;
  String AuthToken;
  String From;
  String* To;
};

class WhatsappNotificator {
  public:
    WhatsappNotificator(WhatsappConfiguration configuration, unsigned int arrayLength) {
      this->configuration = configuration;
      this->arrayLength = arrayLength;
      this->SetupClient();
    }

    void SendMessage(String message) {
      for (byte i = 0; i < this->arrayLength; i++) {
        String payload = "From=$from&To=$to&Body=$body";
        this->configuration.From.replace("+", "%2B");
        this->configuration.To[i].replace("+", "%2B");
        payload.replace("$from", this->configuration.From);
        payload.replace("$to", this->configuration.To[i]);
        payload.replace("$body", message);
        auto httpCode = this->httpClient.POST(payload);
        auto response = this->httpClient.getString();
        httpClient.end();
      }
    }
  private:
    void SetupClient() {
      client.setInsecure();
      String url = "https://api.twilio.com/2010-04-01/Accounts/$Sid/Messages.json";
      url.replace("$Sid", this->configuration.Sid);
      this->httpClient.begin(client, url);
      this->httpClient.addHeader("content-type", "application/x-www-form-urlencoded");
      this->httpClient.setAuthorization(configuration.Sid.c_str(), configuration.AuthToken.c_str());
    }
    unsigned int arrayLength = 0;
    HTTPClient httpClient;
    WhatsappConfiguration configuration;
};

#endif
