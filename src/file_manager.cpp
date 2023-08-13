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

std::unique_ptr<DynamicJsonDocument> FileManager::getFile(const char *path, size_t maxCharBuffer, size_t jsonDocumentSize)
{
  File file = SPIFFS.open(path, "r");
  auto pathStr = String(path);
  if (!file)
  {
    Serial.println("Error opening file at route '" + pathStr + "'");
    return std::unique_ptr<DynamicJsonDocument>(nullptr);
  }
  size_t size = file.size();
  if (size > maxCharBuffer)
  {
    Serial.println("File size at route '" + pathStr + "' is too large");
    return std::unique_ptr<DynamicJsonDocument>(nullptr);
  }
  std::shared_ptr<char[]> buf(new char[size]);
  file.readBytes(buf.get(), size);
  auto pDoc = std::make_unique<DynamicJsonDocument>(jsonDocumentSize);
  DeserializationError error = deserializeJson(*pDoc, buf.get());
  if (error)
  {
    Serial.println("Error parsing file at route '" + pathStr + "'");
    return std::unique_ptr<DynamicJsonDocument>(nullptr);
  }
  return pDoc;
}

