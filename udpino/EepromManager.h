#ifndef EepromManager_h
#define EepromManager_h

#include <Arduino.h>
#include <EEPROM.h>
#define EEPROM_RESSOURCE_ALLOC_SIZE 16
#define EEPROM_RESSOURCE_NUM 6

class EepromManager{
  public:
    EepromManager();
    struct ressource_t{
      uint8_t pin;
      char uri[EEPROM_RESSOURCE_ALLOC_SIZE];
    };
    ressource_t ressources[EEPROM_RESSOURCE_NUM];
    unsigned int ressource_count;
    void initialize();
    void addRessource(uint8_t pin, char* uri);
    void resetMemory();
    void printRessources();
    void setPin(uint8_t pin, char* uri);
  private:
    int last_index;
    boolean uriInUse(char* uri);
    boolean pinInUse(byte pin);
};
#endif
