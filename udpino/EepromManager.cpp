#include <Arduino.h>
#include "EepromManager.h"
#include "Semantic.h"

EepromManager::EepromManager(){

}

void EepromManager::initialize(){
  // render eeprom more usable by storing
  // the ressources in a 2D array
  ressource_count=0;
  int ressource_uri_cursor=0;
  bool on_value=false;
  uint8_t val = 0;
  last_index=-1;
  // variables are separated by a NULL space 0
  for (uint16_t i=0;i<1024;i++) {
    val = EEPROM.read(i);
    if(val!=0){
      if(ressource_count<EEPROM_RESSOURCE_NUM && ressource_uri_cursor<EEPROM_RESSOURCE_ALLOC_SIZE){
        if(on_value) {
          ressources[ressource_count].uri[ressource_uri_cursor]=char(val);
          ressource_uri_cursor++;
        }
        else ressources[ressource_count].pin=val;
      }
      on_value=true;
    } else {
      //count the ressource only when it ends
      if(on_value==true){
        Serial.print(F("Separator at: "));
        Serial.println(i);
        on_value=false;
        while(ressource_uri_cursor<EEPROM_RESSOURCE_ALLOC_SIZE){
          ressources[ressource_count].uri[ressource_uri_cursor]=0;
          ressource_uri_cursor++;
        }
        ressource_count++;
        ressource_uri_cursor=0;
        //update the last WRITABLE index
        last_index=i;
      }
    }
  }
  Serial.print(ressource_count);
  Serial.println(F(" ressource(s) currently in EEPROM"));
  Serial.print(F("Next Ressource will start at: "));
  Serial.println(last_index+1);
}

void EepromManager::addRessource(uint8_t pin, char* uri){
  if(pinInUse(pin) || uriInUse(uri)){
    //not allowed to add the same uri/pin
    Serial.print(F("Can't add "));
    Serial.println(uri);
  } else {
    //add to the 2D array
    Serial.print(F("Adding: "));
    Serial.print(pin);
    Serial.println(uri);
    if(ressource_count<EEPROM_RESSOURCE_NUM)
      ressources[ressource_count].pin=pin;
    for (unsigned int i=0;i<strlen(uri);i++){
      if((ressource_count<EEPROM_RESSOURCE_NUM) && (i<EEPROM_RESSOURCE_ALLOC_SIZE))
        ressources[ressource_count].uri[i]=uri[i];
    }
    //add to eeprom
    EEPROM.write(last_index+1,ressources[ressource_count].pin);
    for (unsigned int i=0; i<strlen(uri);i++){
      EEPROM.write(last_index+2+i,ressources[ressource_count].uri[i]);
    }
    last_index+=strlen(uri)+2;
    ressource_count++;
    Serial.print(F("Next separator should be at "));
    Serial.println(last_index);
    Serial.print(F("Number of ressources"));
    Serial.println(ressource_count);
  }
}

void EepromManager::resetMemory(){
  for (uint16_t i=0;i<1024;i++)
    EEPROM.write(i,0);
}

void EepromManager::printRessources(){
  for(unsigned int i=0;i<ressource_count;i++){
    Serial.print(ressources[i].pin);
    Serial.print(F(","));
    Serial.println(ressources[i].uri);
  }
}

void EepromManager::setPin(uint8_t pin, char* uri){
  for(unsigned int i=0;i<ressource_count;i++){
    if(strcmp(uri,ressources[i].uri)==0 && !pinInUse(pin)){
      ressources[i].pin=pin;
      return;
    } else {
      Serial.print(F("SetPin not allowed for "));
      Serial.println(ressources[i].uri);
    }
  }
}

///////////////////////////////////
// PRIVATE
//////////////////////////////////

boolean EepromManager::uriInUse(char* uri){
  //for every ressource, compare the uri
  for(unsigned int i=0;i<ressource_count;i++){
    unsigned int j=0;
    boolean match=true;
    while(match && j<EEPROM_RESSOURCE_ALLOC_SIZE){
      //if the fields do not match
      //go to the next ressource
      if(ressources[i].uri[j]!=uri[j])
        match=false;
      j++;
    }
    if (match) return true;
  }
  return false;
}

boolean EepromManager::pinInUse(uint8_t pin){
  for(unsigned int i=0;i<ressource_count;i++){
    if(ressources[i].pin==pin) return true;
  }
  return false;
}
