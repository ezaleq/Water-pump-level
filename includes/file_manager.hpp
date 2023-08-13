#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <memory>
#include <ArduinoJson.h>


class FileManager
{
    static void init();
    static std::unique_ptr<DynamicJsonDocument> getFile(const char* path, size_t maxCharBuffer, size_t jsonDocumentSize);
};


class Test

#endif