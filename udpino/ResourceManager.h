#ifndef ResourceManager_h
#define ResourceManager_h

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "Semantic.h"
#define EEPROM_RESOURCE_ALLOC_SIZE 30
#define EEPROM_RESOURCE_NUM 3

struct resource_t{
  const char* json;
  uint8_t pin;
  char id[EEPROM_RESOURCE_ALLOC_SIZE];
};
struct operation_t{
  char uri[EEPROM_RESOURCE_ALLOC_SIZE];
  uint8_t method;
  int expects_index;
  int returns_index;
};

class ResourceManager{
  public:
    ResourceManager();
    operation_t operations[EEPROM_RESOURCE_NUM];
    resource_t resources[EEPROM_RESOURCE_NUM];
    unsigned int resource_count;
    unsigned int operation_count;
    void initialize();
    void addResource(uint8_t pin, char* id, const char* json);
    void addOperation(char * uri, char* method, int expects_index, int returns_index);
    void resetMemory();
    void printResources();
    void printOperations();
    void setPin(uint8_t pin, char* id);
    void parseCapabilities(const char* capabilities, String* chunk);
    int uriInUse(char* uri);
    int idInUse(char* id);
  private:
    void parseChunk(String* chunk);
    int last_index;
    boolean pinInUse(byte pin);
};
#endif
