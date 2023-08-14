#include <memory>
#include <ArduinoJson.h>
#include <FS.h>
#include "../includes/file_manager.hpp"

void FileManager::init()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Error montando el administrador de archivos SPIFFS");
    ESP.restart();
  }
}

void FileManager::parseFile(const char* path, size_t maxCharBuffer, size_t jsonDocumentSize, std::function<void(DynamicJsonDocument&)> parseCallback)
{
  File file = SPIFFS.open(path, "r");
  auto pathStr = String(path);
  if (!file)
  {
    Serial.println("Error opening file at route '" + pathStr + "'");
    return;
  }
  size_t size = file.size();
  if (size > maxCharBuffer)
  {
    Serial.println("File size at route '" + pathStr + "' is too large");
    return;
  }
  std::shared_ptr<char[]> buf(new char[size]);
  file.readBytes(buf.get(), size);
  auto doc = DynamicJsonDocument(jsonDocumentSize);
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error)
  {
    Serial.println("Error parsing file at route '" + pathStr + "'");
    return;
  }
  parseCallback(doc);
}

