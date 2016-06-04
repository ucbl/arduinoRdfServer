#ifndef ResourceManager_h
#define ResourceManager_h

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "Semantic.h"
#define EEPROM_RESOURCE_ALLOC_SIZE 30
#define EEPROM_RESOURCE_NUM 6

struct resource_t{
  const char* json;
  char id[EEPROM_RESOURCE_ALLOC_SIZE];
};
struct operation_t{
  uint8_t pins[6];
  uint8_t pin_count;
  char uri[EEPROM_RESOURCE_ALLOC_SIZE];
  uint8_t method;
  int expects_index;
  int returns_index;
  bool user_defined;
};

class ResourceManager{
  public:
    ResourceManager();
    operation_t operations[EEPROM_RESOURCE_NUM+1];
    resource_t resources[RESOURCE_COUNT+1];
    unsigned int operation_count;
    void initialize_op();
    void initialize_re();
    void addResource(char* id, const char* json, uint8_t index);
    void addEepromEntry(int pin_count, ArduinoJson::JsonArray& pins,const char* uri);
    void addOperation(char* uri, char* method, int expects_index, int returns_index, bool user_defined);
    void refreshEeprom();
    void printResources();
    void printOperations();
    void setPin(uint8_t pin_count, uint8_t* pins, char* uri);
    void parseCapabilities(const char* capabilities, String* chunk);
    int operationInUse(char* uri);
    int operationInUse(const char* uri);
    int resourceInUse(char* id);
    int resourceInUse(const char* id);
  private:
    void completeOperation(int operation_index, char* method, int expects_index, int returns_index, bool user_defined);
    void parseChunk(String* chunk);
    int eeprom_cursor;
    boolean pinInUse(byte pin);
};
#endif
