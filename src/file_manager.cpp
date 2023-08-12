#include "../includes/file_manager.hpp"
#include <FS.h>

void FileManager::init()
{
    if (!SPIFFS.begin())
    {
      Serial.println("Error montando el administrador de archivos SPIFFS");
      return;
    }
}