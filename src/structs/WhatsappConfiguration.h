#ifndef WHATSAPP_CONFIGURATION_H
#define WHATSAPP_CONFIGURATION_H

struct WhatsappConfiguration
{
  String Sid = "";
  String AuthToken = "";
  String From = "";
  String *To = NULL;
  unsigned int ToLength = 0;
};

#endif