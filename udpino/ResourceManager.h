#ifndef ResourceManager_h
#define ResourceManager_h

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "Semantic.h"
#define EEPROM_RESOURCE_ALLOC_SIZE 20
#define EEPROM_RESOURCE_NUM 6

class ResourceManager{
  public:
    ResourceManager();
    const char* capabilities_json;
    struct operation_t{
      char uri[EEPROM_RESOURCE_ALLOC_SIZE];
      int method;
      ressource_t expects;
      ressource_t returns;
    }
    struct resource_t{
      const char* resource_json;
      uint8_t pin;
      char uri[EEPROM_RESOURCE_ALLOC_SIZE];
    };
    operation_t operations[EEPROM_RESOURCE_NUM];
    resource_t resources[EEPROM_RESOURCE_NUM];
    unsigned int resource_count;
    void initialize();
    void addResource(uint8_t pin, char* uri);
    void resetMemory();
    void printResources();
    void setPin(uint8_t pin, char* uri);
    void parseCapabilities(const char* capabilities);
  private:
    int last_index;
    boolean uriInUse(char* uri);
    boolean pinInUse(byte pin);
};
#endif
