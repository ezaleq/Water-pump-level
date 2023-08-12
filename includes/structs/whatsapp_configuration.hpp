#ifndef WHATSAPP_CONFIGURATION_H
#define WHATSAPP_CONFIGURATION_H

struct WhatsappConfiguration
{
  String sid = "";
  String authToken = "";
  String from = "";
  String *to = NULL;
  unsigned int toLength = 0;
};

#endif