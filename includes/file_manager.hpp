#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <ArduinoJson.h>
#include <memory>



class FileManager
{
public:
    static void init();
    static void parseFile(const char* path, size_t maxCharBuffer, size_t jsonDocumentSize, std::function<void(DynamicJsonDocument&)> parseCallback);
    static void saveFile(const char* path, DynamicJsonDocument& jsonDoc);
};

#endif